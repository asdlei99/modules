#include "discovery.h"

#include <pthread.h>

void *broadcast_search_thread(void* arg)
{
    QUdpSocket *udpSocket = (QUdpSocket *)arg;
    QHostAddress broadcastAddress("255.255.255.255"); // Broadcast 地址
    QString data = "this is client";

    int search_times = 20;
    while(search_times-- )
    {
        udpSocket->writeDatagram(data.toUtf8(), broadcastAddress, 3703);
        usleep(200000);
    }

    return arg;
}


void *multicast_search_thread(void* arg)
{
    QUdpSocket *udpSocket = (QUdpSocket *)arg;
    QHostAddress multicastAddress("239.255.255.250"); // Multicast 组的地址
    QString data = "this is client";
    int search_times = 20;
    while(search_times-- )
    {
        udpSocket->writeDatagram(data.toUtf8(), multicastAddress, 3702);
        usleep(1000000);
    }

    return arg;
}



discovery::discovery()
{
    broadcast_udpSocket = new QUdpSocket;
    connect(broadcast_udpSocket, SIGNAL(readyRead()), this, SLOT(broadcast_discovery_msg()), Qt::QueuedConnection);
    multicast_udpSocket = new QUdpSocket;
    connect(multicast_udpSocket, SIGNAL(readyRead()), this, SLOT(multicast_discovery_msg()), Qt::QueuedConnection);


    pthread_t search_thread_id_0;
    pthread_create(&search_thread_id_0, NULL, multicast_search_thread, multicast_udpSocket);
    pthread_detach(search_thread_id_0);

    pthread_t search_thread_id_1;
    pthread_create(&search_thread_id_1, NULL, broadcast_search_thread, broadcast_udpSocket);
    pthread_detach(search_thread_id_1);

}

void discovery::multicast_discovery_msg()
{
    //qDebug() << "多播";
    QByteArray datagram;
    QString discovery_msg;

    while(multicast_udpSocket->hasPendingDatagrams())
    {
        datagram.resize(multicast_udpSocket->pendingDatagramSize());
        multicast_udpSocket->readDatagram(datagram.data(), datagram.size());  //接收数据
    }
    if(datagram.size() > 0)
    {
        discovery_msg.append(datagram.data());
        qDebug() << "多播" << discovery_msg;
    }
}

void discovery::broadcast_discovery_msg()
{
    //qDebug() << "广播";
    QByteArray datagram;
    QString discovery_msg;

    while(broadcast_udpSocket->hasPendingDatagrams())
    {
        datagram.resize(broadcast_udpSocket->pendingDatagramSize());
        broadcast_udpSocket->readDatagram(datagram.data(), datagram.size());  //接收数据
    }
    if(datagram.size() > 0)
    {
        discovery_msg.append(datagram.data());
        qDebug() << "广播" << discovery_msg;
    }
}
