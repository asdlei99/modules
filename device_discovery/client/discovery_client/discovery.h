#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QUdpSocket>


class discovery
{
    Q_OBJECT
public:
    discovery();

    QUdpSocket* broadcast_udpSocket;
    QUdpSocket* multicast_udpSocket;

private slots:
    void multicast_discovery_msg();
    void broadcast_discovery_msg();
};

#endif // DISCOVERY_H
