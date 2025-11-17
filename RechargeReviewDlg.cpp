#include "RechargeReviewDlg.h"
#include "ui_RechargeReviewDlg.h"
#include "DatabaseManager.h"
#include "Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>

RechargeReviewDlg::RechargeReviewDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RechargeReviewDlg)
{
    ui->setupUi(this);
    setWindowTitle("充值审核");
    initializeTable();

    loadPendingRecharges();
}

RechargeReviewDlg::~RechargeReviewDlg()
{
    delete ui;
}

void RechargeReviewDlg::initializeTable()
{
    // 设置表格属性
    ui->rechargeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->rechargeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->rechargeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表头
    QStringList headers = {"记录ID", "用户ID", "用户姓名", "充值金额", "申请时间"};
    ui->rechargeTable->setColumnCount(headers.size());
    ui->rechargeTable->setHorizontalHeaderLabels(headers);

    // 设置列宽
    ui->rechargeTable->setColumnWidth(0, 80);   // 记录ID
    ui->rechargeTable->setColumnWidth(1, 120);  // 用户ID
    ui->rechargeTable->setColumnWidth(2, 100);  // 用户姓名
    ui->rechargeTable->setColumnWidth(3, 100);  // 充值金额
    ui->rechargeTable->horizontalHeader()->setStretchLastSection(true); // 最后一列自适应
}

void RechargeReviewDlg::loadPendingRecharges()
{
    QSqlQuery query;
    query.prepare("SELECT r.record_id, r.user_id, u.name, r.amount, r.created_at "
                  "FROM recharge_records r "
                  "JOIN users u ON r.user_id = u.user_id "
                  "WHERE r.status = 'pending' "
                  "ORDER BY r.created_at ASC");

    // 清空表格
    ui->rechargeTable->setRowCount(0);

    if(!query.exec()) {
        QMessageBox::warning(this, "数据库错误",
                             "查询充值记录失败: " + query.lastError().text());
        return;
    }

    qDebug() << "查询到待审核充值记录:" << query.size();

    int row = 0;
    while(query.next()) {
        ui->rechargeTable->insertRow(row);

        // 记录ID
        QTableWidgetItem *recordIdItem = new QTableWidgetItem(query.value(0).toString());
        recordIdItem->setData(Qt::UserRole, query.value(0).toInt()); // 存储record_id用于后续操作
        ui->rechargeTable->setItem(row, 0, recordIdItem);

        // 用户ID
        ui->rechargeTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));

        // 用户姓名
        ui->rechargeTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));

        // 充值金额
        double amount = query.value(3).toDouble();
        QTableWidgetItem *amountItem = new QTableWidgetItem(QString::number(amount, 'f', 2) + " 元");
        amountItem->setData(Qt::UserRole, amount); // 存储原始金额值
        ui->rechargeTable->setItem(row, 3, amountItem);

        // 申请时间
        QDateTime createTime = query.value(4).toDateTime();
        ui->rechargeTable->setItem(row, 4, new QTableWidgetItem(createTime.toString("yyyy-MM-dd hh:mm:ss")));

        row++;
    }

    if(row == 0) {
        // 如果没有待审核记录，显示提示信息
        ui->rechargeTable->setRowCount(1);
        QTableWidgetItem *infoItem = new QTableWidgetItem("暂无待审核的充值申请");
        infoItem->setTextAlignment(Qt::AlignCenter);
        ui->rechargeTable->setItem(0, 0, infoItem);
        ui->rechargeTable->setSpan(0, 0, 1, 5); // 合并单元格
    }

    qDebug() << "成功加载" << row << "条待审核记录";
}

int RechargeReviewDlg::getSelectedRecordId()
{
    int currentRow = ui->rechargeTable->currentRow();
    if(currentRow >= 0 && currentRow < ui->rechargeTable->rowCount()) {
        QTableWidgetItem *item = ui->rechargeTable->item(currentRow, 0);
        if(item && !item->text().isEmpty() && item->text() != "暂无待审核的充值申请") {
            return item->data(Qt::UserRole).toInt();
        }
    }
    return -1;
}

void RechargeReviewDlg::on_approveBtn_clicked()
{
    int recordId = getSelectedRecordId();
    if(recordId == -1) {
        QMessageBox::warning(this, "审核", "请选择要审核的充值记录");
        return;
    }

    // 获取充值信息
    QSqlQuery query;
    query.prepare("SELECT user_id, amount FROM recharge_records WHERE record_id = ?");
    query.bindValue(0, recordId);

    if(!query.exec() || !query.next()) {
        QMessageBox::warning(this, "审核", "获取充值信息失败");
        return;
    }

    QString userId = query.value(0).toString();
    double amount = query.value(1).toDouble();

    // 开始事务
    QSqlDatabase::database().transaction();

    try {
        // 1. 更新用户余额
        QSqlQuery updateBalance;
        updateBalance.prepare("UPDATE users SET balance = balance + ? WHERE user_id = ?");
        updateBalance.bindValue(0, amount);
        updateBalance.bindValue(1, userId);

        if(!updateBalance.exec()) {
            throw std::runtime_error("更新用户余额失败");
        }

        // 2. 更新充值记录状态
        QSqlQuery updateRecord;
        updateRecord.prepare("UPDATE recharge_records SET status = 'approved', reviewed_at = NOW() WHERE record_id = ?");
        updateRecord.bindValue(0, recordId);

        if(!updateRecord.exec()) {
            throw std::runtime_error("更新充值记录失败");
        }

        // 提交事务
        if(!QSqlDatabase::database().commit()) {
            throw std::runtime_error("事务提交失败");
        }

        Logger::log(Logger::Info,
                    QString("充值审核通过 - 记录ID: %1, 用户: %2, 金额: %3")
                        .arg(recordId).arg(userId).arg(amount));

        QMessageBox::information(this, "审核", "充值申请已通过");
        loadPendingRecharges(); // 刷新列表

    } catch (const std::exception& e) {
        QSqlDatabase::database().rollback();
        QMessageBox::warning(this, "审核失败", e.what());
    }
}

void RechargeReviewDlg::on_rejectBtn_clicked()
{
    int recordId = getSelectedRecordId();
    if(recordId == -1) {
        QMessageBox::warning(this, "审核", "请选择要审核的充值记录");
        return;
    }

    QString reason = QInputDialog::getText(this, "拒绝原因", "请输入拒绝原因:");
    if(reason.isEmpty()) {
        QMessageBox::warning(this, "审核", "必须输入拒绝原因");
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE recharge_records SET status = 'rejected', reason = ?, reviewed_at = NOW() WHERE record_id = ?");
    query.bindValue(0, reason);
    query.bindValue(1, recordId);

    if(query.exec()) {
        Logger::log(Logger::Info,
                    QString("充值审核拒绝 - 记录ID: %1, 原因: %2")
                        .arg(recordId).arg(reason));

        QMessageBox::information(this, "审核", "充值申请已拒绝");
        loadPendingRecharges(); // 刷新列表
    } else {
        QMessageBox::warning(this, "审核失败", "拒绝失败: " + query.lastError().text());
    }
}

void RechargeReviewDlg::on_refreshBtn_clicked()
{
    loadPendingRecharges();
}
