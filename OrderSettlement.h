#ifndef ORDERSETTLEMENT_H
#define ORDERSETTLEMENT_H

#include <QString>

class OrderSettlement
{
public:
    static bool settleOrder(int orderId, QString& errorMsg);
    static bool canSettleOrder(int orderId, QString& errorMsg);

private:
    OrderSettlement() = delete;
    ~OrderSettlement() = delete;
};

#endif // ORDERSETTLEMENT_H
