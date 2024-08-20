package org.example;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

class UE implements Runnable {
    private int sfn = 0;
    boolean sync_check = true;
    int tmp = 0;
    @Override
    public void run() {
        try {
            DatagramSocket socket = new DatagramSocket(9876);
            byte[] buffer = new byte[3];

            while (true) {
                // Tăng SFN mỗi 10ms
                Thread.sleep(10);
                sfn = (sfn + 1) % 1024;
                System.out.println("Local UE updated SFN: " + sfn);
                if(sfn % 8 ==0) {    //cập nhật thông tin sau mỗi 80ms (hay mỗi 8 đơn vị sfn)
//                    System.out.println("UE wake up at UE_sfn: "+sfn);
                    // Nhận bản tin MIB từ gNodeB
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    int receivedSfn = ((buffer[1] & 0xFF) << 8) | (buffer[2] & 0xFF);
                    System.out.println("UE updated MIB with SFN: " + receivedSfn + " and message_id: " + buffer[0]);
//                    if (tmp==88 || (sfn == 8 && sync_check == true)){
//                       System.out.println("UE updated MIB with SFN: " + receivedSfn);
//                        // Cập nhật SFN nếu chưa đồng bộ
//                        if (sfn != receivedSfn) {
//                            sfn = receivedSfn;
//                            System.out.println("UE updated SFN to: " + sfn);
//                        }

//                        // Chỉ cập nhật SFN nếu gói tin nhận được có SFN lớn hơn hoặc bằng SFN hiện tại
//                        if (receivedSfn >= sfn) {
//                            System.out.println("UE updated MIB with SFN: " + receivedSfn);
//                            sfn = receivedSfn;
//                            System.out.println("UE updated SFN to: " + sfn);
//                        } else {
//                            System.out.println("UE ignored MIB with outdated SFN: " + receivedSfn);
//                        }

                        // Cập nhật SFN nếu gói tin nhận được có SFN lớn hơn hoặc bằng SFN hiện tại
//                        if (receivedSfn >= sfn) {
//                            System.out.println("UE updated MIB with SFN: " + receivedSfn);
//                            sfn = receivedSfn;
//                            System.out.println("UE updated SFN to: " + sfn);
//                        } else {
//                            System.out.println("UE ignored MIB with outdated SFN: " + receivedSfn);
//                        }
                    }
                }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}




