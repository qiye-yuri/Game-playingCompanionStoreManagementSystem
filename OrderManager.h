// OrderManager.h
#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QString>

class OrderManager
{
public:
    enum OrderStatus {
        Pending,    // 待接单
        Accepted,   // 已接单
        Completed,  // 已完成
        Cancelled   // 已取消
    };

    static bool canChangeStatus(OrderStatus from, OrderStatus to, const QString& userRole);
    static QString statusToString(OrderStatus status);
    static OrderStatus stringToStatus(const QString& statusStr);

private:
    // 防止实例化
    OrderManager() = delete;
    ~OrderManager() = delete;

    // 禁止拷贝和赋值
    OrderManager(const OrderManager&) = delete;
    OrderManager& operator=(const OrderManager&) = delete;
};

#endif // ORDERMANAGER_H
