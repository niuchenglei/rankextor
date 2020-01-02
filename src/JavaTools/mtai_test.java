package com.suning.aps.mtai.api.test;
import rank.interface.RankInterface;
import com.suning.aps.mtai.api.MTaiLib; 

public class Bank {
    public static void main(String[] args) {
        MTaiLib mtai_imp = new MTaiImp();

        mtai_imp.init_mtai(args[1], 10);

        PredictThread p1 = new PredictThread(1, mtai_imp);
        PredictThread p2 = new PredictThread(2, mtai_imp);
        PredictThread p3 = new PredictThread(3, mtai_imp);
        
        Thread t1 = new Thread(p1);
        Thread t2 = new Thread(p2);
        Thread t3 = new Thread(p3);

        t1.start();
        t2.start();
        t3.start();
    }
}
 
class MTaiImp implements MTaiLib {
}

class PredictThread implements Runnable {
    private Integer idx;
    private MTaiLib mtai;

    public PredictThread(Integer idx, MTaiLib mtai) {
        this.idx = idx; 
        this.mtai = mtai; 
    }

    public void run() {
        RankInterface.Union.Builder builder = RankInterface.Union.newBuilder(); 
        builder.setID(777); 
        builder.setUrl("shiqi"); 

        RankInterface.Union _union = builder.build(); 
        byte[] result = _union.toByteArray() ; 

        int m = 100;
        int i = 0;
        while (i<5) {
            i++;
            try {
                String res = mtai.mtai_predict(result, this.idx);

                RankInterface.ResponseInfo response = RankInterface.ResponseInfo.parseFrom(res); 
                System.out.println("response: " + response.msg);
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }  
        }
    }
}
