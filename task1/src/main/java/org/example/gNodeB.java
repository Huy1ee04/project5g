package org.example;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

class gNodeB implements Runnable {
    private int sfn = 0;

    @Override
    public void run() {
        try {
            DatagramSocket socket = new DatagramSocket();
            InetAddress address = InetAddress.getByName("localhost");


            while (true) {
                // Tăng SFN mỗi 10ms
                Thread.sleep(10);
                sfn = (sfn + 1) % 1024;

                // Gửi bản tin MIB mỗi 80ms
                if (sfn % 8 == 0) {
                    byte[] buffer = new byte[3];
                    buffer[0] = 0x01; // message_id
                    buffer[1] = (byte) (sfn >> 8);
                    buffer[2] = (byte) (sfn & 0xFF);
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, 9876);
                    socket.send(packet);
                    System.out.println("gNodeB sent MIB with SFN: " + sfn);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
