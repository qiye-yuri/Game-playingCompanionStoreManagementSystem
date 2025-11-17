#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "OrderManager.h"
#include "Logger.h"
#include "ValidatorUtils.h"
#include "SecurityUtils.h"
#include "OrderSettlement.h"
#include "RechargeReviewDlg.h"

#include <QtSql/QSqlDatabase>
#include <QMessageBox>
#include <QtSql/QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QFormLayout>
#include <QGroupBox>


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 连接个人信息按钮
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_personalInfoClicked);

    // 连接树形控件点击事件
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::on_treeItemClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setUserInfo(const QString& userId, const QString& role)
{
    m_userId = userId;
    m_userRole = role;
    initNavTreeWidgetByRole();
    refreshBalance();
}

void MainWindow::initNavTreeWidgetByRole(){
    ui->treeWidget->clear();
    ui->treeWidget->setHeaderLabel("功能导航");

    if(m_userRole == "admin") {
        auto userItem = new QTreeWidgetItem({"用户管理"});
        userItem->addChild(new QTreeWidgetItem({"查找用户"}, User_Search));
        userItem->addChild(new QTreeWidgetItem({"添加用户"}, User_Add));
        ui->treeWidget->addTopLevelItem(userItem);

        auto orderItem = new QTreeWidgetItem({"订单管理"});
        orderItem->addChild(new QTreeWidgetItem({"管理订单"}, Order_Manage));
        ui->treeWidget->addTopLevelItem(orderItem);

        QSqlQuery countQuery;
        countQuery.prepare("SELECT COUNT(*) FROM recharge_records WHERE status = 'pending'");
        int pendingCount = 0;
        if(countQuery.exec() && countQuery.next()) {
            pendingCount = countQuery.value(0).toInt();
        }

        QString financeText = "财务管理";
        if(pendingCount > 0) {
            financeText += QString(" (%1待审核)").arg(pendingCount);
        }

        auto rechargeItem = new QTreeWidgetItem({financeText});
        rechargeItem->addChild(new QTreeWidgetItem({"充值审核"}, Recharge_Review));
        ui->treeWidget->addTopLevelItem(rechargeItem);
    }
    else if(m_userRole == "user") {
        auto orderItem = new QTreeWidgetItem({"订单管理"});
        orderItem->addChild(new QTreeWidgetItem({"查看订单"}, Order_View));
        orderItem->addChild(new QTreeWidgetItem({"创建订单"}, Order_Create));
        ui->treeWidget->addTopLevelItem(orderItem);
    }
    else if(m_userRole == "player") {
        auto orderItem = new QTreeWidgetItem({"订单管理"});
        orderItem->addChild(new QTreeWidgetItem({"查看订单"}, Order_View));
        orderItem->addChild(new QTreeWidgetItem({"接单管理"}, Order_Accept));
        ui->treeWidget->addTopLevelItem(orderItem);
    }
    else if(m_userRole == "guest") {
        auto orderItem = new QTreeWidgetItem({"订单查看"});
        orderItem->addChild(new QTreeWidgetItem({"浏览订单"}, Order_View));
        ui->treeWidget->addTopLevelItem(orderItem);
    }
}

void MainWindow::on_treeItemClicked(QTreeWidgetItem* item, int column){
    Q_UNUSED(column)
    NavType type = static_cast<NavType>(item->type());

    switch (type) {
    case User_Search:
        on_userSearch();
        break;
    case User_Add:
        on_userAdd();
        break;
    case Order_View:
        on_orderView();
        break;
    case Order_Create:
        on_orderCreate();
        break;
    case Order_Accept:
        on_orderAccept();
        break;
    case Order_Manage:
        on_orderManage();
        break;
    case Recharge_Review:
        on_rechargeReview();
        break;
    default:
        break;
    }
}

void MainWindow::on_orderView() {
    // 根据角色显示不同的订单数据
    QString queryStr;
    if(m_userRole == "admin") {
        queryStr = "SELECT o.order_id, o.user_id, o.player_id, o.game_type, o.order_status, o.price, o.created_at FROM orders o";
    } else if(m_userRole == "user") {
        queryStr = "SELECT o.order_id, o.user_id, o.player_id, o.game_type, o.order_status, o.price, o.created_at FROM orders o WHERE o.user_id = '" + m_userId + "'";
    } else if(m_userRole == "player") {
        queryStr = "SELECT o.order_id, o.user_id, o.player_id, o.game_type, o.order_status, o.price, o.created_at FROM orders o WHERE o.player_id = '" + m_userId + "' OR o.player_id IS NULL";
    } else {
        queryStr = "SELECT o.order_id, o.game_type, o.order_status, o.price, o.created_at FROM orders o WHERE o.order_status = 'completed'";
    }

    QSqlQuery query(queryStr);
    if(!query.exec()){
        QMessageBox::warning(this,"查询错误",query.lastError().text());
        return;
    }

    // 创建或重置模型
    if(!m_order_model){
        m_order_model = new QStandardItemModel;
    } else {
        m_order_model->clear();
    }

    // 设置表头
    if(m_userRole == "admin") {
        m_order_model->setHorizontalHeaderLabels({"订单ID", "用户ID", "打手ID", "游戏类型", "状态", "价格", "创建时间"});
    } else if(m_userRole == "guest") {
        m_order_model->setHorizontalHeaderLabels({"订单ID", "游戏类型", "状态", "价格", "创建时间"});
    } else {
        m_order_model->setHorizontalHeaderLabels({"订单ID", "用户ID", "打手ID", "游戏类型", "状态", "价格", "创建时间"});
    }

    // 填充数据
    while(query.next()){
        QList<QStandardItem*> item_list;
        for(int i = 0; i < query.record().count(); i++){
            item_list.push_back(new QStandardItem(query.value(i).toString()));
        }
        m_order_model->appendRow(item_list);
    }

    // 设置到表格
    QTableView* tableView = new QTableView();
    tableView->setModel(m_order_model);
    tableView->horizontalHeader()->setStretchLastSection(true);

    // 清除stackedWidget并添加新页面
    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(tableView);
}

void MainWindow::on_orderCreate() {
    createOrderPage();
}

void MainWindow::on_orderAccept() {
    createOrderAcceptPage();
}

void MainWindow::on_orderManage() {
    createOrderManagePage();
}

void MainWindow::on_userSearch() {
    QSqlQuery query("SELECT user_id, name, role, phone, email, balance, created_at FROM users");
    if(!query.exec()){
        QMessageBox::warning(this,"查询错误",query.lastError().text());
        return;
    }

    if(!m_user_model){
        m_user_model = new QStandardItemModel;
    } else {
        m_user_model->clear();
    }

    m_user_model->setHorizontalHeaderLabels({"用户ID", "姓名", "角色", "电话", "邮箱", "余额", "注册时间"});

    while(query.next()){
        QList<QStandardItem*> item_list;
        for(int i = 0; i < query.record().count(); i++){
            item_list.push_back(new QStandardItem(query.value(i).toString()));
        }
        m_user_model->appendRow(item_list);
    }

    QTableView* tableView = new QTableView();
    tableView->setModel(m_user_model);
    tableView->horizontalHeader()->setStretchLastSection(true);

    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(tableView);
}

void MainWindow::on_userAdd() {
    createUserAddPage();
}

void MainWindow::on_userProfile() {
    createUserProfilePage();
}

void MainWindow::on_personalInfoClicked() {
    createUserProfilePage();
}

void MainWindow::createOrderPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    // 游戏类型选择
    QHBoxLayout* gameLayout = new QHBoxLayout();
    gameLayout->addWidget(new QLabel("游戏类型:"));
    QComboBox* gameCombo = new QComboBox();
    gameCombo->addItems({"英雄联盟", "王者荣耀", "三角洲行动", "CS:GO", "绝地求生", "瓦罗兰特", "其他"});
    gameLayout->addWidget(gameCombo);
    layout->addLayout(gameLayout);

    // 价格输入
    QHBoxLayout* priceLayout = new QHBoxLayout();
    priceLayout->addWidget(new QLabel("价格:"));
    QLineEdit* priceEdit = new QLineEdit();
    priceEdit->setPlaceholderText("请输入价格");
    priceLayout->addWidget(priceEdit);
    layout->addLayout(priceLayout);

    // 描述输入
    QHBoxLayout* descLayout = new QHBoxLayout();
    descLayout->addWidget(new QLabel("订单描述:"));
    QTextEdit* descEdit = new QTextEdit();
    descEdit->setMaximumHeight(100);
    descLayout->addWidget(descEdit);
    layout->addLayout(descLayout);

    // 提交按钮
    QPushButton* submitBtn = new QPushButton("创建订单");
    layout->addWidget(submitBtn);

    connect(submitBtn, &QPushButton::clicked, [this, gameCombo, priceEdit, descEdit](){
        QString gameType = gameCombo->currentText();
        QString price = priceEdit->text();
        QString description = descEdit->toPlainText();

        if(gameType.isEmpty() || price.isEmpty()){
            QMessageBox::warning(this, "创建订单", "请填写完整信息");
            return;
        }

        QSqlQuery query;
        query.prepare("INSERT INTO orders (user_id, game_type, price, description) VALUES (?, ?, ?, ?)");
        query.bindValue(0, m_userId);
        query.bindValue(1, gameType);
        query.bindValue(2, price);
        query.bindValue(3, description);

        if(query.exec()){
            QMessageBox::information(this, "创建订单", "订单创建成功!");
            Logger::log(Logger::Info,
                        QString("创建订单 - 游戏类型: %1, 价格: %2")
                        .arg(gameType).arg(price),
                        m_userId);
            priceEdit->clear();
            descEdit->clear();
        } else {
            QMessageBox::warning(this, "创建订单", "创建失败: " + query.lastError().text());
        }
    });

    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(page);
}

void MainWindow::createOrderAcceptPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    // 查询待接订单
    QSqlQuery query("SELECT order_id, user_id, game_type, price, description FROM orders WHERE order_status = 'pending'");

    if(!m_order_model){
        m_order_model = new QStandardItemModel;
    } else {
        m_order_model->clear();
    }

    m_order_model->setHorizontalHeaderLabels({"订单ID", "用户ID", "游戏类型", "价格", "描述", "操作"});

    while(query.next()){
        QList<QStandardItem*> item_list;
        for(int i = 0; i < 5; i++){ // 前5个字段
            item_list.push_back(new QStandardItem(query.value(i).toString()));
        }

        // 添加接单按钮
        QStandardItem* actionItem = new QStandardItem("接单");
        actionItem->setData(query.value(0).toString(), Qt::UserRole); // 存储order_id
        item_list.push_back(actionItem);

        m_order_model->appendRow(item_list);
    }

    QTableView* tableView = new QTableView();
    tableView->setModel(m_order_model);
    tableView->horizontalHeader()->setStretchLastSection(true);

    // 连接接单按钮点击
    connect(tableView, &QTableView::clicked, [this, tableView](const QModelIndex& index){
        if(index.column() == 5){ // 操作列
            QString orderId = index.data(Qt::UserRole).toString();

            QSqlQuery query;
            query.prepare("UPDATE orders SET player_id = ?, order_status = 'accepted' WHERE order_id = ?");
            query.bindValue(0, m_userId);
            query.bindValue(1, orderId);

            if(query.exec()){
                QMessageBox::information(this, "接单", "接单成功!");
                Logger::log(Logger::Info,
                            QString("接单成功 - 订单ID: %1")
                            .arg(orderId),
                            m_userId);
                on_orderAccept(); // 刷新页面
            } else {
                QMessageBox::warning(this, "接单", "接单失败: " + query.lastError().text());
            }
        }
    });

    layout->addWidget(tableView);

    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(page);
}

void MainWindow::createOrderManagePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    QSqlQuery query("SELECT order_id, user_id, player_id, game_type, order_status, price, description FROM orders");

    if(!m_order_model){
        m_order_model = new QStandardItemModel;
    } else {
        m_order_model->clear();
    }

    m_order_model->setHorizontalHeaderLabels({"订单ID", "用户ID", "打手ID", "游戏类型", "状态", "价格", "描述", "操作"});

    while(query.next()){
        QList<QStandardItem*> item_list;
        for(int i = 0; i < 7; i++){
            item_list.push_back(new QStandardItem(query.value(i).toString()));
        }

        QStandardItem* actionItem = new QStandardItem("管理");
        actionItem->setData(query.value(0).toString(), Qt::UserRole);
        item_list.push_back(actionItem);

        m_order_model->appendRow(item_list);
    }

    QTableView* tableView = new QTableView();
    tableView->setModel(m_order_model);
    tableView->horizontalHeader()->setStretchLastSection(true);

    connect(tableView, &QTableView::clicked, [this, tableView](const QModelIndex& index){
        if(index.column() == 7){
            QString orderId = index.data(Qt::UserRole).toString();
            QString currentStatus = m_order_model->item(index.row(), 4)->text();

            // 状态管理对话框
            QDialog dialog(this);
            dialog.setWindowTitle("订单管理");
            QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

            QComboBox* statusCombo = new QComboBox();
            statusCombo->addItems({"pending", "accepted", "completed", "cancelled"});
            statusCombo->setCurrentText(currentStatus);
            dialogLayout->addWidget(statusCombo);

            QPushButton* confirmBtn = new QPushButton("确认修改");
            dialogLayout->addWidget(confirmBtn);

            connect(confirmBtn, &QPushButton::clicked, [&, orderId, index](){
                QString newStatus = statusCombo->currentText();
                QString currentStatus = m_order_model->item(index.row(), 4)->text();

                // 使用 OrderManager 验证状态转换是否合法
                OrderManager::OrderStatus current = OrderManager::stringToStatus(currentStatus);
                OrderManager::OrderStatus newStatusEnum = OrderManager::stringToStatus(newStatus);

                if (!OrderManager::canChangeStatus(current, newStatusEnum, m_userRole)) {
                    QMessageBox::warning(this, "订单管理",
                                         QString("不允许从状态 '%1' 转换到 '%2'").arg(currentStatus).arg(newStatus));
                    return;
                }

                // 如果是完成订单，进行结算
                if (newStatus == "completed" && currentStatus != "completed") {
                    QString errorMsg;
                    if (OrderSettlement::settleOrder(orderId.toInt(), errorMsg)) {
                        QMessageBox::information(this, "订单管理", "订单完成结算成功!");
                        m_order_model->item(index.row(), 4)->setText(newStatus);

                        // 记录日志
                        Logger::log(Logger::Info,
                                    QString("订单完成结算 - 订单ID: %1").arg(orderId),
                                    m_userId);

                        dialog.close();
                        refreshBalance(); // 刷新余额显示
                    } else {
                        QMessageBox::warning(this, "订单结算失败", errorMsg);
                        return;
                    }
                } else {
                    // 其他状态更新
                    QSqlQuery query;
                    query.prepare("UPDATE orders SET order_status = ? WHERE order_id = ?");
                    query.bindValue(0, newStatus);
                    query.bindValue(1, orderId);

                    if(query.exec()){
                        QMessageBox::information(this, "订单管理", "状态更新成功!");
                        m_order_model->item(index.row(), 4)->setText(newStatus);
                        dialog.close();
                    } else {
                        QMessageBox::warning(this, "订单管理", "更新失败: " + query.lastError().text());
                    }
                }
            });

            dialog.exec();
        }
    });

    layout->addWidget(tableView);
    showPage(page);
}

void MainWindow::createUserAddPage() {
    QWidget* page = new QWidget();
    QFormLayout* layout = new QFormLayout(page);

    QLineEdit* userIdEdit = new QLineEdit();
    QLineEdit* passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    QComboBox* roleCombo = new QComboBox();
    roleCombo->addItems({"user", "player", "admin"});
    QLineEdit* nameEdit = new QLineEdit();
    QLineEdit* phoneEdit = new QLineEdit();
    QLineEdit* emailEdit = new QLineEdit();
    QLineEdit* balanceEdit = new QLineEdit();
    balanceEdit->setText("0");

    layout->addRow("用户ID:", userIdEdit);
    layout->addRow("密码:", passwordEdit);
    layout->addRow("角色:", roleCombo);
    layout->addRow("姓名:", nameEdit);
    layout->addRow("电话:", phoneEdit);
    layout->addRow("邮箱:", emailEdit);
    layout->addRow("余额:", balanceEdit);

    QPushButton* submitBtn = new QPushButton("添加用户");
    layout->addRow(submitBtn);

    connect(submitBtn, &QPushButton::clicked, [this, userIdEdit, passwordEdit, roleCombo, nameEdit, phoneEdit, emailEdit, balanceEdit](){
        QString userId = userIdEdit->text();
        QString password = passwordEdit->text();
        QString role = roleCombo->currentText();
        QString name = nameEdit->text();
        QString phone = phoneEdit->text();
        QString email = emailEdit->text();
        QString balance = balanceEdit->text();

        if(userId.isEmpty() || password.isEmpty()){
            QMessageBox::warning(this, "添加用户", "用户ID和密码不能为空");
            return;
        }

        // 验证用户ID格式
        if(!ValidatorUtils::validateUserId(userId)) {
            QMessageBox::warning(this, "添加用户", "用户ID格式不正确（4-20位字母数字）");
            return;
        }

        // 验证密码格式
        if(!ValidatorUtils::validatePassword(password)) {
            QMessageBox::warning(this, "添加用户", "密码格式不正确（至少6位，包含字母和数字）");
            return;
        }

        // 验证邮箱格式（如果填写了邮箱）
        if(!email.isEmpty() && !ValidatorUtils::validateEmail(email)) {
            QMessageBox::warning(this, "添加用户", "邮箱格式不正确");
            return;
        }

        // 验证手机格式（如果填写了手机）
        if(!phone.isEmpty() && !ValidatorUtils::validatePhone(phone)) {
            QMessageBox::warning(this, "添加用户", "手机号码格式不正确");
            return;
        }

        // 检查用户是否已存在
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT user_id FROM users WHERE user_id = ?");
        checkQuery.bindValue(0, userId);

        if(checkQuery.exec() && checkQuery.next()) {
            QMessageBox::warning(this, "添加用户", "用户ID已存在");
            return;
        }

        // 对密码进行哈希处理
        QString hashedPassword = SecurityUtils::hashPassword(password);

        QSqlQuery query;
        query.prepare("INSERT INTO users (user_id, password, role, name, phone, email, balance) VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.bindValue(0, userId);
        query.bindValue(1, hashedPassword);  // 使用哈希后的密码
        query.bindValue(2, role);
        query.bindValue(3, name);
        query.bindValue(4, phone);
        query.bindValue(5, email);
        query.bindValue(6, balance);

        if(query.exec()){
            QMessageBox::information(this, "添加用户", "用户添加成功!");
            Logger::log(Logger::Info,
                        QString("添加新用户 - 用户ID: %1, 角色: %2")
                            .arg(userId).arg(role),
                        m_userId);
            // 清空输入框
            userIdEdit->clear();
            passwordEdit->clear();
            nameEdit->clear();
            phoneEdit->clear();
            emailEdit->clear();
            balanceEdit->setText("0");
        } else {
            QMessageBox::warning(this, "添加用户", "添加失败: " + query.lastError().text());
        }
    });

    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(page);
}

void MainWindow::createUserProfilePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    // 查询用户信息
    QSqlQuery query;
    query.prepare("SELECT user_id, name, role, phone, email, balance FROM users WHERE user_id = ?");
    query.bindValue(0, m_userId);

    if(!query.exec() || !query.next()){
        QMessageBox::warning(this, "个人信息", "获取用户信息失败");
        return;
    }

    QFormLayout* formLayout = new QFormLayout();

    // 用户ID（不可编辑）
    QLabel* userIdLabel = new QLabel(query.value(0).toString());
    formLayout->addRow("用户ID:", userIdLabel);

    // 姓名（可编辑）
    QLineEdit* nameEdit = new QLineEdit(query.value(1).toString());
    formLayout->addRow("姓名:", nameEdit);

    // 角色（不可编辑）
    QLabel* roleLabel = new QLabel(query.value(2).toString());
    formLayout->addRow("角色:", roleLabel);

    // 电话（可编辑）
    QLineEdit* phoneEdit = new QLineEdit(query.value(3).toString());
    phoneEdit->setPlaceholderText("请输入手机号码");
    formLayout->addRow("电话:", phoneEdit);

    // 邮箱（可编辑）
    QLineEdit* emailEdit = new QLineEdit(query.value(4).toString());
    emailEdit->setPlaceholderText("请输入邮箱地址");
    formLayout->addRow("邮箱:", emailEdit);

    // 余额（不可编辑）
    QLabel* balanceLabel = new QLabel(query.value(5).toString() + " 元");
    formLayout->addRow("余额:", balanceLabel);

    layout->addLayout(formLayout);

    // 修改密码区域
    QGroupBox* passwordGroup = new QGroupBox("修改密码");
    QFormLayout* passwordLayout = new QFormLayout(passwordGroup);

    QLineEdit* currentPasswordEdit = new QLineEdit();
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("请输入当前密码");
    passwordLayout->addRow("当前密码:", currentPasswordEdit);

    QLineEdit* newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码");
    passwordLayout->addRow("新密码:", newPasswordEdit);

    QLineEdit* confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请确认新密码");
    passwordLayout->addRow("确认密码:", confirmPasswordEdit);

    layout->addWidget(passwordGroup);

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* saveProfileBtn = new QPushButton("保存个人信息");
    QPushButton* changePasswordBtn = new QPushButton("修改密码");
    QPushButton* cancelBtn = new QPushButton("取消");

    buttonLayout->addWidget(saveProfileBtn);
    buttonLayout->addWidget(changePasswordBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addLayout(buttonLayout);

    // 如果是用户角色，显示充值功能
    if(m_userRole == "user" || m_userRole == "player"){
        QGroupBox* rechargeGroup = new QGroupBox("账户充值");
        QHBoxLayout* rechargeLayout = new QHBoxLayout(rechargeGroup);

        QLineEdit* amountEdit = new QLineEdit();
        amountEdit->setPlaceholderText("充值金额");
        QPushButton* rechargeBtn = new QPushButton("充值");

        rechargeLayout->addWidget(amountEdit);
        rechargeLayout->addWidget(rechargeBtn);

        layout->addWidget(rechargeGroup);

        connect(rechargeBtn, &QPushButton::clicked, [this, amountEdit](){
            QString amountStr = amountEdit->text().trimmed();
            bool ok;
            double amount = amountStr.toDouble(&ok);

            if(!ok || amount <= 0){
                QMessageBox::warning(this, "充值", "请输入有效的充值金额");
                return;
            }

            if(amount > 10000) {
                QMessageBox::warning(this, "充值", "单次充值金额不能超过10000元");
                return;
            }

            // 插入充值记录（待审核状态）
            QSqlQuery query;
            query.prepare("INSERT INTO recharge_records (user_id, amount, status) VALUES (?, ?, 'pending')");
            query.bindValue(0, m_userId);
            query.bindValue(1, amount);

            if(query.exec()){
                Logger::log(Logger::Info,
                            QString("提交充值申请 - 金额: %1元").arg(amount),
                            m_userId);

                // 显示成功提示
                QMessageBox::information(this, "充值申请",
                                         QString("充值申请已提交!\n金额: %1元\n状态: 等待管理员审核\n\n审核通过后余额将自动更新").arg(amount));
                amountEdit->clear();

            } else {
                QMessageBox::warning(this, "充值失败", "提交充值申请失败: " + query.lastError().text());
                qDebug() << "充值申请插入失败:" << query.lastError().text();
            }
        });
    }

    // 保存个人信息
    connect(saveProfileBtn, &QPushButton::clicked, [this, nameEdit, phoneEdit, emailEdit](){
        QString name = nameEdit->text().trimmed();
        QString phone = phoneEdit->text().trimmed();
        QString email = emailEdit->text().trimmed();

        // 验证输入
        if(name.isEmpty()){
            QMessageBox::warning(this, "保存失败", "姓名不能为空");
            return;
        }

        if(!phone.isEmpty() && !ValidatorUtils::validatePhone(phone)){
            QMessageBox::warning(this, "保存失败", "手机号码格式不正确");
            return;
        }

        if(!email.isEmpty() && !ValidatorUtils::validateEmail(email)){
            QMessageBox::warning(this, "保存失败", "邮箱格式不正确");
            return;
        }

        QSqlQuery query;
        query.prepare("UPDATE users SET name = ?, phone = ?, email = ? WHERE user_id = ?");
        query.bindValue(0, name);
        query.bindValue(1, phone);
        query.bindValue(2, email);
        query.bindValue(3, m_userId);

        if(query.exec()){
            Logger::log(Logger::Info,
                        QString("更新个人信息 - 姓名: %1, 电话: %2, 邮箱: %3")
                            .arg(name).arg(phone).arg(email),
                        m_userId);

            QMessageBox::information(this, "保存成功", "个人信息已更新");
        } else {
            QMessageBox::warning(this, "保存失败", "更新失败: " + query.lastError().text());
        }
    });

    // 修改密码
    connect(changePasswordBtn, &QPushButton::clicked, [this, currentPasswordEdit, newPasswordEdit, confirmPasswordEdit](){
        QString currentPassword = currentPasswordEdit->text();
        QString newPassword = newPasswordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();

        if(currentPassword.isEmpty() || newPassword.isEmpty() || confirmPassword.isEmpty()){
            QMessageBox::warning(this, "修改密码", "所有密码字段都不能为空");
            return;
        }

        if(newPassword != confirmPassword){
            QMessageBox::warning(this, "修改密码", "两次输入的新密码不一致");
            return;
        }

        if(!ValidatorUtils::validatePassword(newPassword)){
            QMessageBox::warning(this, "修改密码", "新密码格式不正确（至少6位，包含字母和数字）");
            return;
        }

        // 验证当前密码
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT password FROM users WHERE user_id = ?");
        checkQuery.bindValue(0, m_userId);

        if(!checkQuery.exec() || !checkQuery.next()){
            QMessageBox::warning(this, "修改密码", "验证当前密码失败");
            return;
        }

        QString storedHash = checkQuery.value(0).toString();
        if(!SecurityUtils::verifyPassword(currentPassword, storedHash)){
            QMessageBox::warning(this, "修改密码", "当前密码不正确");
            return;
        }

        // 更新密码
        QString newHashedPassword = SecurityUtils::hashPassword(newPassword);
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET password = ? WHERE user_id = ?");
        updateQuery.bindValue(0, newHashedPassword);
        updateQuery.bindValue(1, m_userId);

        if(updateQuery.exec()){
            Logger::log(Logger::Info, "密码修改成功", m_userId);
            QMessageBox::information(this, "修改密码", "密码修改成功");

            // 清空密码输入框
            currentPasswordEdit->clear();
            newPasswordEdit->clear();
            confirmPasswordEdit->clear();
        } else {
            QMessageBox::warning(this, "修改密码", "密码修改失败: " + updateQuery.lastError().text());
        }
    });

    // 取消按钮
    connect(cancelBtn, &QPushButton::clicked, [this](){
        createUserProfilePage(); // 重新加载页面，放弃修改
    });

    while(ui->stackedWidget->count() > 0){
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    ui->stackedWidget->addWidget(page);
}

void MainWindow::refreshBalance() {
    if(m_userRole == "guest") return;

    QSqlQuery query;
    query.prepare("SELECT balance FROM users WHERE user_id = ?");
    query.bindValue(0, m_userId);

    if(query.exec() && query.next()){
        double balance = query.value(0).toDouble();
        ui->pushButton->setText(QString("个人信息 (余额: %1 元)").arg(balance));
    }
}

void MainWindow::showPage(QWidget* page)
{
    // 安全清理旧页面
    clearStackedWidget();

    if(page) {
        ui->stackedWidget->addWidget(page);
        ui->stackedWidget->setCurrentWidget(page);
    }
}

void MainWindow::clearStackedWidget()
{
    while(ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);

        // 安全删除，避免悬空指针
        if(widget && widget->parent() == ui->stackedWidget) {
            widget->deleteLater();
        }
    }
}

void MainWindow::on_rechargeReview()
{
    RechargeReviewDlg *dlg = new RechargeReviewDlg(this);
    dlg->exec();
    delete dlg;
}
