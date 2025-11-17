#include "OrderManager.h"
#include <QMap>
#include <QList>
#include <QString>

bool OrderManager::canChangeStatus(OrderStatus from, OrderStatus to, const QString& userRole)
{
    // 定义状态流转规则 - 使用静态局部变量避免初始化问题
    static QMap<OrderStatus, QList<OrderStatus>> adminRules;
    static QMap<OrderStatus, QList<OrderStatus>> playerRules;
    static QMap<OrderStatus, QList<OrderStatus>> userRules;

    // 延迟初始化，避免静态初始化顺序问题
    if (adminRules.isEmpty()) {
        // 管理员规则：可以执行所有状态转换
        adminRules[Pending] = {Accepted, Cancelled};
        adminRules[Accepted] = {Completed, Cancelled, Pending};
        adminRules[Completed] = {Pending}; // 特殊情况：重新开启已完成订单
        adminRules[Cancelled] = {Pending}; // 重新开启已取消订单

        // 打手规则：只能接单和完成订单
        playerRules[Pending] = {Accepted};
        playerRules[Accepted] = {Completed};
        playerRules[Completed] = {};
        playerRules[Cancelled] = {};

        // 用户规则：只能取消待接单的订单
        userRules[Pending] = {Cancelled};
        userRules[Accepted] = {};
        userRules[Completed] = {};
        userRules[Cancelled] = {};
    }

    const QMap<OrderStatus, QList<OrderStatus>>* rules = nullptr;

    if (userRole == "admin") {
        rules = &adminRules;
    } else if (userRole == "player") {
        rules = &playerRules;
    } else if (userRole == "user") {
        rules = &userRules;
    } else {
        return false; // 游客或其他未知角色无权限
    }

    return rules->value(from).contains(to);
}

QString OrderManager::statusToString(OrderStatus status)
{
    static QMap<OrderStatus, QString> statusMap;

    if (statusMap.isEmpty()) {
        statusMap[Pending] = "pending";
        statusMap[Accepted] = "accepted";
        statusMap[Completed] = "completed";
        statusMap[Cancelled] = "cancelled";
    }

    return statusMap.value(status, "unknown");
}

OrderManager::OrderStatus OrderManager::stringToStatus(const QString& statusStr)
{
    static QMap<QString, OrderStatus> stringMap;

    if (stringMap.isEmpty()) {
        stringMap["pending"] = Pending;
        stringMap["accepted"] = Accepted;
        stringMap["completed"] = Completed;
        stringMap["cancelled"] = Cancelled;
    }

    return stringMap.value(statusStr.toLower(), Pending);
}
