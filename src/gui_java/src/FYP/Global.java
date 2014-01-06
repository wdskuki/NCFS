/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package FYP;

//import com.sun.corba.se.impl.orbutil.concurrent.Mutex;
import java.awt.Dimension;
import java.util.ArrayList;

import java.util.concurrent.locks.*;

/**
 *
 * @author user
 */
public class Global {
    public static Top_Level_Process TopLevelProcess = null;
    public static Recovery_Process RecoveryProcess = null;
    public static Content_Browser ContentBrowser = null;
    public static Setup_Details SetupDetails = null;
    public static Disk_Details DiskDetails = null;
    public static Progress_Bar ProgressBar = null;
    public static ArrayList DiskDetailsChild = new ArrayList();
    public static Welcome Wel = null;
    public static Network_Layer sd;
    public static Dimension screenSize;
    public static Dimension frameSize;

    //public static Mutex lock = new Mutex();
    public static Lock lock = new ReentrantLock();

    public static Rename RenameText = null;
    public static String Rename_name = null;

    //public static boolean testing = false;
    public static boolean testing = true;
}
