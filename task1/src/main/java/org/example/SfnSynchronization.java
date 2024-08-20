package org.example;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class SfnSynchronization {
    public static void main(String[] args) {
        Thread gNodeBThread = new Thread(new gNodeB());
        Thread UEThread = new Thread(new UE());

        gNodeBThread.start();
        UEThread.start();
    }
}
