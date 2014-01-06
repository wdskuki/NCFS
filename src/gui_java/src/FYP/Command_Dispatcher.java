/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package FYP;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author user
 */
public class Command_Dispatcher implements Runnable{

    Command_Dispatcher(){
    }

    public void run(){
        Byte inModule;
        while(true){
            try{
                inModule = Global.sd.ReadByte();
            } catch (IOException e){
                continue;
            }
            //System.out.println("inModule " + inModule);
            switch(inModule){
                case 0:     Global.TopLevelProcess.ProcessHandler();break;
                case 1:     Global.DiskDetails.ProcessHandler();break;
                case 2:     Global.RecoveryProcess.ProcessHandler();break;
                case 3:try {
                    Global.ContentBrowser.ProcessHandler();
                } catch (InterruptedException ex) {
                    Logger.getLogger(Command_Dispatcher.class.getName()).log(Level.SEVERE, null, ex);
                }break;
                //case 4:     Global.ProgressBar.ProcessHandler();break;
                default:    continue;
            }
        }
    }
}