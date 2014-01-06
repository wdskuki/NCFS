/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * About.java
 *
 * Created on 2010年10月5日, 下午07:55:34
 */

package FYP;

//import   java.awt.*;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Point;
import java.util.Timer;
import java.util.TimerTask;
import javax.swing.Icon;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *
 * @author Andrew Kong
 */
public class Progress_Bar extends javax.swing.JDialog {

    /** Creates new form About */
    //public Progress_Bar(java.awt.Frame parent, boolean modal) {
    public Progress_Bar(Dimension GUI_screenSize, Dimension GUI_frameSize) {
        //super(parent, modal);
        super();
        setTitle("ECFS Progress");
        initComponents();

        // Center in the screen
        Dimension frameSize = getSize();
        setLocation(new Point((GUI_screenSize.width + GUI_frameSize.width - frameSize.width) / 2,
                              (GUI_screenSize.height - GUI_frameSize.height - frameSize.height) / 2));
        //setLocation(new Point((screenSize.width - frameSize.width) / 2,
        //                      (screenSize.height - frameSize.height) / 2));
    }

    private static final GridBagConstraints gbc;
    static {
        gbc = new GridBagConstraints();
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        gbc.fill = GridBagConstraints.BOTH;
        gbc.anchor = GridBagConstraints.NORTHWEST;
    }

    public static JPanel wrapInBackgroundImage(JComponent component,
            Icon backgroundIcon) {
        return wrapInBackgroundImage(
                component,
                backgroundIcon,
                JLabel.TOP,
                JLabel.LEADING);
    }

    public static JPanel wrapInBackgroundImage(JComponent component,
            Icon backgroundIcon,
            int verticalAlignment,
            int horizontalAlignment) {

        // make the passed in swing component transparent
        component.setOpaque(false);

        // create wrapper JPanel
        JPanel backgroundPanel = new JPanel(new GridBagLayout());

        // add the passed in swing component first to ensure that it is in front
        backgroundPanel.add(component, gbc);

        // create a label to paint the background image
        JLabel backgroundImage = new JLabel(backgroundIcon);

        // set minimum and preferred sizes so that the size of the image
        // does not affect the layout size
        //backgroundImage.setPreferredSize(new Dimension(1,1));
        //backgroundImage.setMinimumSize(new Dimension(1,1));
        backgroundImage.setPreferredSize(new Dimension(1,1));

        // align the image as specified.
        backgroundImage.setVerticalAlignment(verticalAlignment);
        backgroundImage.setHorizontalAlignment(horizontalAlignment);

        // add the background label
        backgroundPanel.add(backgroundImage, gbc);

        // return the wrapper
        return backgroundPanel;
    }

    public void setLabel(String FileName, String statement, String FileLocation){
        this.File_name.setText(FileName);
        this.statement1.setText(statement);
        this.Location.setText(FileLocation);

        jProgressBar1.setStringPainted(true);
        Timer timer = new Timer();
        task = new MyTask();
        timer.schedule(task, 0, 1000);
    }

    public void setNumber(long length, int percentage)
    {
        Global_length = length;
        jProgressBar1.setValue(percentage);
    }

    public class MyTask extends java.util.TimerTask{
    @Override
        public void run() {
            int no = count;
            int minute = 0;
            int second = 0;
            long rate = 0;
            String tempString = new String();
            if (no <= 1)
                Time.setText(Integer.toString(no)+" second");
            else if (no >= 60)
            {
                minute = no/60;
                second = no%60;
                if (minute == 1)
                {
                    if (second <= 1){
                        //Time.setText(Integer.toString(minute)+" minute " + Integer.toString(second)+" second");
                        tempString = Integer.toString(minute)+" minute " + Integer.toString(second)+" second";
                        Time.setText(tempString);
                        Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+tempString);
                    }
                    else{
                        //Time.setText(Integer.toString(minute)+" minute " + Integer.toString(second)+" seconds");
                        tempString = Integer.toString(minute)+" minute " + Integer.toString(second)+" seconds";
                        Time.setText(tempString);
                        Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+tempString);
                    }
                }
                else
                {
                    if (second <= 1){
                        //Time.setText(Integer.toString(minute)+" minutes " + Integer.toString(second)+" second");
                        tempString = Integer.toString(minute)+" minutes " + Integer.toString(second)+" second";
                        Time.setText(tempString);
                        Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+tempString);
                    }
                    else{
                        //Time.setText(Integer.toString(minute)+" minutes " + Integer.toString(second)+" seconds");
                        tempString = Integer.toString(minute)+" minutes " + Integer.toString(second)+" seconds";
                        Time.setText(tempString);
                        Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+tempString);
                    }
                }
            }
            else{
                //Time.setText(Integer.toString(no)+" seconds");
                tempString = Integer.toString(no)+" seconds";
                Time.setText(tempString);
                Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+tempString);
            }

            //Global.ContentBrowser.ElapsedTime.setText("Elapsed Time: "+Integer.toString(no) + " seconds");

            if (count != 0)
            {
                rate = Global_length / count;
                if (rate >= 1024 * 1024){
                    //Transfer_rate.setText((float)Math.round((float)rate/(1024*1024)*10)/10 + " MB/second");
                    tempString = (float)Math.round((float)rate/(1024*1024)*10)/10 + " MB/second";
                    Transfer_rate.setText(tempString);
                    Global.ContentBrowser.Throughput.setText("Throughput: "+tempString);
                }
                else if (rate >= 1024){
                    //Transfer_rate.setText((float)Math.round((float)rate/1024*10)/10 + " KB/second");
                    tempString = (float)Math.round((float)rate/1024*10)/10 + " KB/second";
                    Transfer_rate.setText(tempString);
                    Global.ContentBrowser.Throughput.setText("Throughput: "+tempString);
                }
                else{
                    //if(rate < 1024)
                    //Transfer_rate.setText(rate + " B/second");
                    tempString = rate + " B/second";
                    Transfer_rate.setText(tempString);
                    Global.ContentBrowser.Throughput.setText("Throughput: "+tempString);
                }
            }
            else{
                //Transfer_rate.setText("--- MB/second");
                tempString = "--- MB/second";
                Transfer_rate.setText(tempString);
                Global.ContentBrowser.Throughput.setText("Throughput: "+tempString);
            }
            count++;

            //Global.ContentBrowser.Throughput.setText("Throughput: "+rate+ " B/second");
            Global.ContentBrowser.DataSize.setText("Data size: "+(float)Math.round(Global_length/(1024*1024)*10)/10+ " MB");
        }
    }

    public void setCancel() throws InterruptedException
    {
        Global_length = 0;
        task.cancel();
        Thread.currentThread();
	Thread.sleep(3000);
        setVisible(false);
        dispose();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel_About = new javax.swing.JPanel();
        File_name = new javax.swing.JLabel();
        Cancel = new javax.swing.JButton();
        statement1 = new javax.swing.JLabel();
        jProgressBar1 = new javax.swing.JProgressBar();
        Location = new javax.swing.JLabel();
        statement3 = new javax.swing.JLabel();
        Time = new javax.swing.JLabel();
        statement5 = new javax.swing.JLabel();
        Transfer_rate = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);

        File_name.setText("File name");

        Cancel.setText("Cancel");
        Cancel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CancelActionPerformed(evt);
            }
        });

        statement1.setText("Download to:");

        Location.setText("C:/Location");

        statement3.setText("Timer:");

        Time.setText("0 second");

        statement5.setText("Transfer rate:");

        Transfer_rate.setText("--- MB/second");

        javax.swing.GroupLayout jPanel_AboutLayout = new javax.swing.GroupLayout(jPanel_About);
        jPanel_About.setLayout(jPanel_AboutLayout);
        jPanel_AboutLayout.setHorizontalGroup(
            jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel_AboutLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(File_name, javax.swing.GroupLayout.PREFERRED_SIZE, 320, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jProgressBar1, javax.swing.GroupLayout.PREFERRED_SIZE, 340, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel_AboutLayout.createSequentialGroup()
                        .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(statement1)
                            .addComponent(statement5)
                            .addComponent(statement3))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(Transfer_rate)
                            .addComponent(Time)
                            .addComponent(Location))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel_AboutLayout.createSequentialGroup()
                .addContainerGap(297, Short.MAX_VALUE)
                .addComponent(Cancel)
                .addContainerGap())
        );
        jPanel_AboutLayout.setVerticalGroup(
            jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel_AboutLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(File_name)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jProgressBar1, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(statement1)
                    .addComponent(Location))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(statement3)
                    .addComponent(Time))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel_AboutLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(statement5)
                    .addComponent(Transfer_rate))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(Cancel)
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel_About, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel_About, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void CancelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CancelActionPerformed
        // TODO add your handling code here:
        //task.cancel();
        //setVisible(false);
        //dispose();
}//GEN-LAST:event_CancelActionPerformed

    /**
    * @param args the command line arguments
    */
    
    TimerTask task;
    int count = 0;

    long Global_length = 0;

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton Cancel;
    private javax.swing.JLabel File_name;
    private javax.swing.JLabel Location;
    private javax.swing.JLabel Time;
    private javax.swing.JLabel Transfer_rate;
    private javax.swing.JPanel jPanel_About;
    private javax.swing.JProgressBar jProgressBar1;
    private javax.swing.JLabel statement1;
    private javax.swing.JLabel statement3;
    private javax.swing.JLabel statement5;
    // End of variables declaration//GEN-END:variables
}
