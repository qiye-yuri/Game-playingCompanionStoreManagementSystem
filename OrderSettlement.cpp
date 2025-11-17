#include "OrderSettlement.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>

bool OrderSettlement::canSettleOrder(int orderId, QString& errorMsg)
{
    QSqlQuery query;
    query.prepare("SELECT o.order_status, o.price, o.user_id, o.player_id, "
                  "u.balance as user_balance, p.balance as player_balance "
                  "FROM orders o "
                  "LEFT JOIN users u ON o.user_id = u.user_id "
                  "LEFT JOIN users p ON o.player_id = p.user_id "
                  "WHERE o.order_id = ?");
    query.bindValue(0, orderId);

    if(!query.exec() || !query.next()) {
        errorMsg = "订单不存在或查询失败";
        return false;
    }

    QString status = query.value(0).toString();
    double price = query.value(1).toDouble();
    QString userId = query.value(2).toString();
    QString playerId = query.value(3).toString();
    double userBalance = query.value(4).toDouble();

    if(status != "accepted") {
        errorMsg = "只有已接单的订单才能完成结算";
        return false;
    }

    if(playerId.isEmpty()) {
        errorMsg = "订单没有指定打手，无法结算";
        return false;
    }

    if(userBalance < price) {
        errorMsg = "用户余额不足，无法完成订单";
        return false;
    }

    return true;
}

bool OrderSettlement::settleOrder(int orderId, QString& errorMsg)
{
    // 检查是否可以结算
    if(!canSettleOrder(orderId, errorMsg)) {
        return false;
    }

    // 获取订单信息
    QSqlQuery query;
    query.prepare("SELECT price, user_id, player_id FROM orders WHERE order_id = ?");
    query.bindValue(0, orderId);

    if(!query.exec() || !query.next()) {
        errorMsg = "获取订单信息失败";
        return false;
    }

    double price = query.value(0).toDouble();
    QString userId = query.value(1).toString();
    QString playerId = query.value(2).toString();

    // 开始事务
    QSqlDatabase::database().transaction();

    try {
        // 1. 从用户余额扣除
        QSqlQuery deductQuery;
        deductQuery.prepare("UPDATE users SET balance = balance - ? WHERE user_id = ?");
        deductQuery.bindValue(0, price);
        deductQuery.bindValue(1, userId);

        if(!deductQuery.exec()) {
            throw std::runtime_error("扣除用户余额失败: " + deductQuery.lastError().text().toStdString());
        }

        // 2. 给打手余额增加
        QSqlQuery addQuery;
        addQuery.prepare("UPDATE users SET balance = balance + ? WHERE user_id = ?");
        addQuery.bindValue(0, price);
        addQuery.bindValue(1, playerId);

        if(!addQuery.exec()) {
            throw std::runtime_error("增加打手余额失败: " + addQuery.lastError().text().toStdString());
        }

        // 3. 更新订单状态为已完成
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE orders SET order_status = 'completed' WHERE order_id = ?");
        updateQuery.bindValue(0, orderId);

        if(!updateQuery.exec()) {
            throw std::runtime_error("更新订单状态失败: " + updateQuery.lastError().text().toStdString());
        }

        // 提交事务
        if(!QSqlDatabase::database().commit()) {
            throw std::runtime_error("事务提交失败");
        }

        return true;

    } catch (const std::exception& e) {
        // 回滚事务
        QSqlDatabase::database().rollback();
        errorMsg = e.what();
        return false;
    }
}
