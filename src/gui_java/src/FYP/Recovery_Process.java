/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * Disk_Details.java
 *
 * Created on 2010/10/24, 下午 08:16:05
 */

package FYP;

import java.io.IOException;
import java.util.Timer;
import javax.swing.GroupLayout;
import javax.swing.SwingUtilities;
import static java.lang.Thread.*;
import java.util.TimerTask;
import javax.swing.Icon;
import javax.swing.JLabel;

/**
 *
 * @author user
 */
public class Recovery_Process extends javax.swing.JPanel {

    /** Creates new form Disk_Details */
    public Recovery_Process() {
        initComponents();

        Raid.setText(null);
        jLabel6.setVisible(false);
        arrow.setVisible(false);
        arrow1.setVisible(false);
        New_disk.setVisible(false);
        Timer_Label.setVisible(false);
        Throughput_Label.setVisible(false);
        DataSize_Label.setVisible(false);

        //Raid.setText("Raid " + Global.SetupDetails.Raid);

        File_icon[0] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper1.gif")); // NOI18N
        File_icon[1] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper3.gif")); // NOI18N
        File_icon[2] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper5.gif")); // NOI18N
        File_icon[3] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper9.gif")); // NOI18N
        File_icon[4] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper20.gif")); // NOI18N
        File_icon[5] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper22.gif")); // NOI18N
        File_icon[6] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper24-25.gif")); // NOI18N
        File_icon[7] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/paper27.gif")); // NOI18N

        Replace_icon[0] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace1.gif")); // NOI18N
        Replace_icon[1] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace2.gif")); // NOI18N
        Replace_icon[2] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace3.gif")); // NOI18N
        Replace_icon[3] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace4.gif")); // NOI18N
        Replace_icon[4] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace5.gif")); // NOI18N
        Replace_icon[5] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace6.gif")); // NOI18N
        Replace_icon[6] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace7.gif")); // NOI18N
        Replace_icon[7] = new javax.swing.ImageIcon(getClass().getResource("/Resourses/replace8.gif")); // NOI18N

        //arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/arrow-up-icon-w.gif")));
        //arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/arrow-up-icon-w.gif")));

        for (int i = 0; i < 8; i++)
        {
            Copy_file[i] = new javax.swing.JLabel();
            //Copy_file[i].setText("file");
            Copy_file[i].setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
            Copy_file[i].setIcon(Replace_icon[i]);
        }

        //jPanel10.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel10Layout = new javax.swing.GroupLayout(jPanel10);
            jPanel10.setLayout(jPanel10Layout);
            jPanel10Layout.setHorizontalGroup(
                jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[0])
            );
            jPanel10Layout.setVerticalGroup(
                jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel10Layout.createSequentialGroup()
                    .addComponent(Copy_file[0])
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

       // jPanel12.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel12Layout = new javax.swing.GroupLayout(jPanel12);
            jPanel12.setLayout(jPanel12Layout);
            jPanel12Layout.setHorizontalGroup(
                jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[1])
            );
            jPanel12Layout.setVerticalGroup(
                jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel12Layout.createSequentialGroup()
                    .addComponent(Copy_file[1])
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

        //jPanel13.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel13Layout = new javax.swing.GroupLayout(jPanel13);
            jPanel13.setLayout(jPanel13Layout);
            jPanel13Layout.setHorizontalGroup(
                jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[2])
            );
            jPanel13Layout.setVerticalGroup(
                jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel13Layout.createSequentialGroup()
                    .addComponent(Copy_file[2])
                    .addContainerGap(15, Short.MAX_VALUE))
            );

        //jPanel14.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel14Layout = new javax.swing.GroupLayout(jPanel14);
            jPanel14.setLayout(jPanel14Layout);
            jPanel14Layout.setHorizontalGroup(
                jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[3])
            );
            jPanel14Layout.setVerticalGroup(
                jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel14Layout.createSequentialGroup()
                    .addComponent(Copy_file[3])
                    .addContainerGap(15, Short.MAX_VALUE))
            );

        //jPanel15.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel15Layout = new javax.swing.GroupLayout(jPanel15);
            jPanel15.setLayout(jPanel15Layout);
            jPanel15Layout.setHorizontalGroup(
                jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[4])
            );
            jPanel15Layout.setVerticalGroup(
                jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel15Layout.createSequentialGroup()
                    .addComponent(Copy_file[4])
                    .addContainerGap(15, Short.MAX_VALUE))
            );

        //jPanel16.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel16Layout = new javax.swing.GroupLayout(jPanel16);
            jPanel16.setLayout(jPanel16Layout);
            jPanel16Layout.setHorizontalGroup(
                jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[5])
            );
            jPanel16Layout.setVerticalGroup(
                jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel16Layout.createSequentialGroup()
                    .addComponent(Copy_file[5])
                    .addContainerGap(15, Short.MAX_VALUE))
            );

        //jPanel17.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel17Layout = new javax.swing.GroupLayout(jPanel17);
            jPanel17.setLayout(jPanel17Layout);
            jPanel17Layout.setHorizontalGroup(
                jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[6])
            );
            jPanel17Layout.setVerticalGroup(
                jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel17Layout.createSequentialGroup()
                    .addComponent(Copy_file[6])
                    .addContainerGap(15, Short.MAX_VALUE))
            );

        //jPanel18.setBackground(new java.awt.Color(220,234,247));

            javax.swing.GroupLayout jPanel18Layout = new javax.swing.GroupLayout(jPanel18);
            jPanel18.setLayout(jPanel18Layout);
            jPanel18Layout.setHorizontalGroup(
                jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(Copy_file[7])
            );
            jPanel18Layout.setVerticalGroup(
                jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(jPanel18Layout.createSequentialGroup()
                    .addComponent(Copy_file[7])
                    .addContainerGap(15, Short.MAX_VALUE))
            );
    }

    public void ProcessHandler(){
        byte inbyte;
        String str;
        try{
            inbyte = Global.sd.ReadByte();
        } catch (IOException e) {
            return;
        }
        
        if (inbyte == 0)
        {
            str = Global.sd.ReadString();
            if (!str.equals("/"))
                {
                    String[] temp = temp_str;
                    temp_str = new String[temp_str.length + 1];
                    System.arraycopy(temp, 0, temp_str, 0, temp_str.length - 1);
                    temp_str[temp_str.length-1] = str;
                }
            else
                SwingUtilities.invokeLater(new Start(temp_str.length - 1, temp_str));
        }
        else if (inbyte == 1)
            SwingUtilities.invokeLater(new Stop());
        else return;
    }

     /**
     * @brief   Update the GUI after the event-dispatching thread
     */
    private class Start implements Runnable{
        private int no_of_disk;
        private String[] str;
        Start(int _no_of_disk, String[] _str){
            no_of_disk = _no_of_disk;
            str = _str;
        }
        public void run() {
            Global.SetupDetails.change_dev_name(no_of_disk, str);
            setLabel(no_of_disk, str);
            start_to_run();
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane5 = new javax.swing.JScrollPane();
        jPanel_recovery_process = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        New_disk = new javax.swing.JLabel();
        arrow = new javax.swing.JLabel();
        jPanel8 = new javax.swing.JPanel();
        jPanel10 = new javax.swing.JPanel();
        jPanel12 = new javax.swing.JPanel();
        jPanel13 = new javax.swing.JPanel();
        jPanel14 = new javax.swing.JPanel();
        jPanel15 = new javax.swing.JPanel();
        jPanel16 = new javax.swing.JPanel();
        jPanel17 = new javax.swing.JPanel();
        jPanel18 = new javax.swing.JPanel();
        Raid = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        arrow1 = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        Timer_Label = new javax.swing.JLabel();
        Time = new javax.swing.JLabel();
        Throughput_Label = new javax.swing.JLabel();
        Throughput = new javax.swing.JLabel();
        DataSize_Label = new javax.swing.JLabel();
        DataSize = new javax.swing.JLabel();

        setMaximumSize(new java.awt.Dimension(640, 480));
        setMinimumSize(new java.awt.Dimension(640, 480));
        setPreferredSize(new java.awt.Dimension(640, 480));

        jScrollPane5.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane5.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        jPanel_recovery_process.setBackground(new java.awt.Color(220, 234, 247));
        jPanel_recovery_process.setMaximumSize(new java.awt.Dimension(600, 400));
        jPanel_recovery_process.setMinimumSize(new java.awt.Dimension(600, 400));
        jPanel_recovery_process.setPreferredSize(new java.awt.Dimension(600, 400));

        jPanel4.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 168, Short.MAX_VALUE)
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        New_disk.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/disk_128.gif"))); // NOI18N

        arrow.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif"))); // NOI18N

        jPanel8.setBackground(new java.awt.Color(220, 234, 247));

        jPanel10.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel10Layout = new javax.swing.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 48, Short.MAX_VALUE)
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 58, Short.MAX_VALUE)
        );

        jPanel12.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel12Layout = new javax.swing.GroupLayout(jPanel12);
        jPanel12.setLayout(jPanel12Layout);
        jPanel12Layout.setHorizontalGroup(
            jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 34, Short.MAX_VALUE)
        );
        jPanel12Layout.setVerticalGroup(
            jPanel12Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 58, Short.MAX_VALUE)
        );

        jPanel13.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel13Layout = new javax.swing.GroupLayout(jPanel13);
        jPanel13.setLayout(jPanel13Layout);
        jPanel13Layout.setHorizontalGroup(
            jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 44, Short.MAX_VALUE)
        );
        jPanel13Layout.setVerticalGroup(
            jPanel13Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 62, Short.MAX_VALUE)
        );

        jPanel14.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel14Layout = new javax.swing.GroupLayout(jPanel14);
        jPanel14.setLayout(jPanel14Layout);
        jPanel14Layout.setHorizontalGroup(
            jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 33, Short.MAX_VALUE)
        );
        jPanel14Layout.setVerticalGroup(
            jPanel14Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        jPanel15.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel15Layout = new javax.swing.GroupLayout(jPanel15);
        jPanel15.setLayout(jPanel15Layout);
        jPanel15Layout.setHorizontalGroup(
            jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 19, Short.MAX_VALUE)
        );
        jPanel15Layout.setVerticalGroup(
            jPanel15Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        jPanel16.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel16Layout = new javax.swing.GroupLayout(jPanel16);
        jPanel16.setLayout(jPanel16Layout);
        jPanel16Layout.setHorizontalGroup(
            jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 31, Short.MAX_VALUE)
        );
        jPanel16Layout.setVerticalGroup(
            jPanel16Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        jPanel17.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel17Layout = new javax.swing.GroupLayout(jPanel17);
        jPanel17.setLayout(jPanel17Layout);
        jPanel17Layout.setHorizontalGroup(
            jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 28, Short.MAX_VALUE)
        );
        jPanel17Layout.setVerticalGroup(
            jPanel17Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        jPanel18.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel18Layout = new javax.swing.GroupLayout(jPanel18);
        jPanel18.setLayout(jPanel18Layout);
        jPanel18Layout.setHorizontalGroup(
            jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 32, Short.MAX_VALUE)
        );
        jPanel18Layout.setVerticalGroup(
            jPanel18Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 63, Short.MAX_VALUE)
        );

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel10, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel12, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel13, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel14, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel15, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel16, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel17, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel18, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel8Layout.createSequentialGroup()
                .addContainerGap(29, Short.MAX_VALUE)
                .addComponent(jPanel15, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(75, 75, 75))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel8Layout.createSequentialGroup()
                .addContainerGap(39, Short.MAX_VALUE)
                .addComponent(jPanel16, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(65, 65, 65))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel8Layout.createSequentialGroup()
                .addContainerGap(71, Short.MAX_VALUE)
                .addComponent(jPanel17, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(33, 33, 33))
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGap(18, 18, 18)
                .addComponent(jPanel14, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 11, Short.MAX_VALUE)
                .addComponent(jPanel18, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel13, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(jPanel12, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(jPanel10, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(93, Short.MAX_VALUE))
        );

        Raid.setFont(new java.awt.Font("新細明體", 1, 12));
        Raid.setText("Raid");

        jLabel6.setHorizontalAlignment(javax.swing.SwingConstants.TRAILING);
        jLabel6.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Database_icon.png"))); // NOI18N

        arrow1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif"))); // NOI18N

        jLabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Recovery_Process.png"))); // NOI18N

        Timer_Label.setText("Elapsed Time:");

        Time.setText("          ");

        Throughput.setText("          ");

        DataSize.setText("          ");

        javax.swing.GroupLayout jPanel_recovery_processLayout = new javax.swing.GroupLayout(jPanel_recovery_process);
        jPanel_recovery_process.setLayout(jPanel_recovery_processLayout);
        jPanel_recovery_processLayout.setHorizontalGroup(
            jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(Throughput_Label)
                            .addComponent(DataSize_Label))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(DataSize, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(Throughput, javax.swing.GroupLayout.DEFAULT_SIZE, 103, Short.MAX_VALUE)))
                    .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(Timer_Label)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(Time, javax.swing.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE))
                    .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                            .addContainerGap()
                            .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                .addComponent(arrow, javax.swing.GroupLayout.PREFERRED_SIZE, 168, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(arrow1, javax.swing.GroupLayout.PREFERRED_SIZE, 168, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(jLabel6)))
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel_recovery_processLayout.createSequentialGroup()
                            .addContainerGap()
                            .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                        .addGap(90, 90, 90)
                        .addComponent(Raid))
                    .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel1)
                    .addComponent(New_disk))
                .addContainerGap())
        );
        jPanel_recovery_processLayout.setVerticalGroup(
            jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel_recovery_processLayout.createSequentialGroup()
                        .addComponent(Raid)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                                .addComponent(jLabel6)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(arrow1)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(arrow)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addComponent(New_disk, javax.swing.GroupLayout.Alignment.TRAILING))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 113, Short.MAX_VALUE)
                .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel1)
                    .addGroup(jPanel_recovery_processLayout.createSequentialGroup()
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(Timer_Label)
                            .addComponent(Time))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(Throughput_Label)
                            .addComponent(Throughput))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel_recovery_processLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(DataSize_Label)
                            .addComponent(DataSize))))
                .addContainerGap())
        );

        jScrollPane5.setViewportView(jPanel_recovery_process);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jScrollPane5, javax.swing.GroupLayout.DEFAULT_SIZE, 628, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jScrollPane5, javax.swing.GroupLayout.DEFAULT_SIZE, 468, Short.MAX_VALUE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    public void setLabel(int no_of_disk, String[] str){
        disk_num = no_of_disk;
        Disk = new javax.swing.JLabel[no_of_disk];

        if (Global.SetupDetails.Raid.equals("1000")){
            Raid.setText("MBR exact repair");
        }
        else if (Global.SetupDetails.Raid.equals("2000")) {
            Raid.setText("MSR exact repair");
        }
        else{
            Raid.setText("Raid " + Global.SetupDetails.Raid);
        }
        jLabel6.setVisible(true);
        arrow.setVisible(true);
        arrow1.setVisible(true);
        New_disk.setVisible(true);
        Timer_Label.setVisible(true);
        Throughput_Label.setVisible(false);
        DataSize_Label.setVisible(false);

        New_disk.setText(str[no_of_disk]);
        New_disk.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        New_disk.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);

        for (int i = 0; i < no_of_disk; i++)
        {
            Disk[i] = new javax.swing.JLabel();
            Disk[i].setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/disk_48.gif"))); // NOI18N
            Disk[i].setText(str[i]);
            Disk[i].setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
            Disk[i].setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        }

        int no_of_row = no_of_disk/3;
        if (no_of_disk % 3 != 0)
            no_of_row++;

        //jPanel4 = new javax.swing.JPanel();
        //jPanel4.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);

        GroupLayout.ParallelGroup PG = jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING);
        GroupLayout.SequentialGroup[] SG = new GroupLayout.SequentialGroup[no_of_row];
        for (int j = 0; j < no_of_row; j++)
        {
             SG[j] = jPanel4Layout.createSequentialGroup();
             PG.addGroup(SG[j]);
        }
        int count = -1;
        for (int i = 0; i < no_of_disk; i++)
        {
            if (i % 3 == 0)
                count++;
            SG[count].addComponent(Disk[i]);
            ////////////////////////////////
            SG[count].addGap(10);
        }
        jPanel4Layout.setHorizontalGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING).addGroup(PG));

        GroupLayout.SequentialGroup SGV = jPanel4Layout.createSequentialGroup();
        GroupLayout.ParallelGroup[] PGV = new GroupLayout.ParallelGroup[no_of_row];
        for (int j = 0; j < no_of_row; j++)
        {
             PGV[j] = jPanel4Layout.createParallelGroup();
             SGV.addGroup(PGV[j]);
        }
        count = -1;
        for (int i = 0; i < no_of_disk; i++)
        {
            if (i % 3 == 0)
                count++;
            PGV[count].addComponent(Disk[i]);
        }
        jPanel4Layout.setVerticalGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING).addGroup(SGV));

        temp_str = null;
    }

    private class Stop implements Runnable{
        /*Stop(){
        }*/
        public void run() {
            stop();
        }
    }

    public void stop(){
        float size_in_MB;
        task.cancel();
        task2.cancel();
        arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
        arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
        /*Raid.setText(null);
        jLabel6.setVisible(false);
        arrow.setVisible(false);
        arrow1.setVisible(false);
        New_disk.setVisible(false);
        for(int i = 0; i < disk_num; ++i){
            Disk[i].setVisible(false);
        }*/
        Throughput_Label.setVisible(true);
        DataSize_Label.setVisible(true);
        size_in_MB = (float)Math.round(Global.SetupDetails.global_disk_size/(1024*1024)*10)/10;
        Throughput_Label.setText("Throughput: "+(float)Math.round((size_in_MB/counter)*10)/10 +" MB/s");
        DataSize_Label.setText("Data Size: "+ size_in_MB +" MB");
    }
    
    public void start_to_run(){
        Timer timer = new Timer();
        task = new MyTask();
        task2 = new MyTask2();
        timer.scheduleAtFixedRate(task, 0, 1600);
        timer.scheduleAtFixedRate(task2, 0, 1000);
    }

    public class MyTask extends java.util.TimerTask{
    @Override
        public void run() {
            // TODO Auto-generated method stub
            int memory = 7;
            for (int i = 0; i < 8; i++)
            {
                System.out.println(i);
                Copy_file[i].setIcon(File_icon[i]);
                Copy_file[memory].setIcon(Replace_icon[memory]);
                memory = i;
                try
                {
                    if (i == 1)
                        {
                            arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/arrow-up-icon-w.gif")));
                            arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
                        }
                    if (i == 4 || i == 7)
                        {
                            arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
                            arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
                        }
                    else if (i == 5)
                        {
                            arrow1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/arrow-up-icon-w.gif")));
                            arrow.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Up_arrow.gif")));
                        }
                    if (i < 4)
                        sleep(150);
                    else if (i < 6)
                        sleep(200);
                    else sleep(300);
                }
                    catch (InterruptedException ex)
                 {
                    System.out.println("interrupted");
                }
            }
            Copy_file[memory].setIcon(Replace_icon[memory]);
        }
    }
    
    public class MyTask2 extends java.util.TimerTask{
    @Override
        public void run() {
            int no = counter;
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
                    if (second <= 1)
                        Time.setText(Integer.toString(minute)+" minute " + Integer.toString(second)+" second");
                    else
                        Time.setText(Integer.toString(minute)+" minute " + Integer.toString(second)+" seconds");
                }
                else
                {
                    if (second <= 1)
                        Time.setText(Integer.toString(minute)+" minutes " + Integer.toString(second)+" second");
                    else
                        Time.setText(Integer.toString(minute)+" minutes " + Integer.toString(second)+" seconds");
                }
            }
            else Time.setText(Integer.toString(no)+" seconds");

            counter++;
        }
    }

    String[] temp_str = {};
    
    JLabel[] Copy_file = new javax.swing.JLabel[8];
    Icon[] File_icon = new Icon[8];
    Icon[] Replace_icon = new Icon[8];
    TimerTask task;
    JLabel[] Disk;
    int disk_num = 0;
    long Global_length = 0;

    TimerTask task2;
    int counter;

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel DataSize;
    private javax.swing.JLabel DataSize_Label;
    private javax.swing.JLabel New_disk;
    private javax.swing.JLabel Raid;
    private javax.swing.JLabel Throughput;
    private javax.swing.JLabel Throughput_Label;
    private javax.swing.JLabel Time;
    private javax.swing.JLabel Timer_Label;
    private javax.swing.JLabel arrow;
    private javax.swing.JLabel arrow1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel12;
    private javax.swing.JPanel jPanel13;
    private javax.swing.JPanel jPanel14;
    private javax.swing.JPanel jPanel15;
    private javax.swing.JPanel jPanel16;
    private javax.swing.JPanel jPanel17;
    private javax.swing.JPanel jPanel18;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel_recovery_process;
    private javax.swing.JScrollPane jScrollPane5;
    // End of variables declaration//GEN-END:variables
}
