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

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import javax.swing.GroupLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *
 * @author user
 */
public class Setup_Details extends javax.swing.JPanel {

    /** Creates new form Setup_Details */
    public Setup_Details() {
        initComponents();

        DataInputStream in = null;
        BufferedReader br = null;
        String strLine;
        String file;

        // Open the file that is the first
        // command line parameter
        file = System.getProperty("user.dir");
        if (file.charAt(0) == '/')
            file = file + "/raid_setting";
        else file = file + "\\raid_setting";
        File file2 = new File(file);
        System.out.println(file2.length());     //in terms of byte
        int no_of_disk;
        String[] dev_name;
        String[] free_size;

        try{
        FileInputStream raid_setting = new FileInputStream(file);

        in = new DataInputStream(raid_setting);
        br = new BufferedReader(new InputStreamReader(in));

        strLine = global_param_str(br.readLine(), "disk_total_num");
        no_of_disk = Integer.parseInt(strLine);

        String strLine2 = global_param_str(br.readLine(), "data_disk_num");
        // TODO Handle the data_disk_num

        dev_name = new String[no_of_disk];
        free_size = new String[no_of_disk];
        Disk_total_num_text.setText(strLine);
        Disk_block_size_text.setText(global_param_str(br.readLine(), "disk_block_size"));
        Raid = global_param_str(br.readLine(), "disk_raid_type");
        if (Raid.equals("1000")){
            Disk_raid_type_text.setText("MBR (Minimum Bandwidth Regenerating)");
        }
        else{
            Disk_raid_type_text.setText(Raid);
        }
        br.readLine();
        //Read File Line By Line
        int count = 0;
        while ((strLine = br.readLine()) != null)   {
            // Print the content on the console
            dev_name[count] = global_param_str(strLine.substring(2), "dev_name");
            br.readLine();
            free_size[count] = global_param_str(br.readLine().substring(2), "free_size");
            count++;
        }
        global_disk_size = Long.valueOf(free_size[0]);
        //Close the input stream
        raid_setting.close();
        in.close();
        br.close();
        
        }catch (Exception e){//Catch exception if any
            System.err.println("Error: " + e.getMessage());
            return;
        }
        setLabel(no_of_disk, dev_name, free_size);
    }

    private void setLabel(int no_of_disk, String[] dev_name, String[] free_size){
        Disk = new javax.swing.JLabel[no_of_disk];
        JLabel[] Free_Size = new javax.swing.JLabel[no_of_disk];
        JPanel[] jPanel = new javax.swing.JPanel[no_of_disk];

        for (int i = 0; i < no_of_disk; i++)
        {
            Disk_Details_Child DD = new Disk_Details_Child();
            DD.setFreeOffset(0);
            DD.setFreeSize(free_size[i]);
            DD.setTotalSize(free_size[i]);
            //if(Global.DiskDetails == null)
            //    System.out.println("Alert");
            Global.DiskDetails.add_Tab("Disk " + i, DD);

            Disk_Details_Overall_Child DDO = new Disk_Details_Overall_Child();
            Global.DiskDetails.add_Overall_Child("Disk "+ i, DDO);

            jPanel[i] = new javax.swing.JPanel();
            //jPanel[i].setBackground(new java.awt.Color(255, 255, 255));
            jPanel[i].setBackground(new java.awt.Color(220,234,247));
            Disk[i] = new javax.swing.JLabel();
            Free_Size[i] = new javax.swing.JLabel();
            Disk[i].setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/disk_48.gif"))); // NOI18N
            Disk[i].setText(dev_name[i]);
            Disk[i].setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
            Disk[i].setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
            Free_Size[i].setText(free_size[i] + " bytes");
            Free_Size[i].setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
            Free_Size[i].setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        }


        int no_of_row = no_of_disk/3;
        if (no_of_disk % 3 != 0)
            no_of_row++;
        
        for (int i = 0; i < no_of_disk; i++)
        {
        javax.swing.GroupLayout jPanelLayout = new javax.swing.GroupLayout(jPanel[i]);
        jPanel[i].setLayout(jPanelLayout);
        jPanelLayout.setHorizontalGroup(
            jPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(Disk[i])
                    .addComponent(Free_Size[i]))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanelLayout.setVerticalGroup(
            jPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(Disk[i])
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(Free_Size[i])
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        }

        //jPanel4.setBackground(new java.awt.Color(255, 255, 255));
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
            SG[count].addComponent(jPanel[i]);
        }
        jPanel4Layout.setHorizontalGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING).addGroup(PG));

        GroupLayout.SequentialGroup SGV = jPanel4Layout.createSequentialGroup();
        GroupLayout.ParallelGroup[] PGV = new GroupLayout.ParallelGroup[no_of_row*2];
        for (int j = 0; j < no_of_row*2; j++)
        {
             PGV[j] = jPanel4Layout.createParallelGroup();
             SGV.addGroup(PGV[j]);
        }
        count = -1;
        for (int i = 0; i < no_of_disk; i++)
        {
            if (i % 3 == 0)
                count++;
            PGV[count*2].addComponent(jPanel[i]);
        }
        jPanel4Layout.setVerticalGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING).addGroup(SGV));
    }

    public void change_dev_name(int no_of_disk, String[] str){
        for (int i = 0; i < no_of_disk; i++)
        {
            boolean check = false;
            for (int j = 0; j < no_of_disk; j++)
            {
                if (Disk[i].getText().equals(str[j]))
                {
                    check = true;
                    break;
                }
            }
            if (check == false)
            {
                Disk[i].setText(str[no_of_disk]);
                return;
            }
        }
    }

public static boolean is_param(String line, String param_str){
    return line.startsWith(param_str);
}

public static String global_param_str(String line, String param_str){
    if (is_param((line), (param_str))){
        return line.substring(param_str.length() + 1);
    }
    return null;
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
        Setup_details = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        Disk_total_num = new javax.swing.JLabel();
        Disk_total_num_text = new javax.swing.JLabel();
        Disk_block_size = new javax.swing.JLabel();
        Disk_block_size_text = new javax.swing.JLabel();
        Disk_raid_type = new javax.swing.JLabel();
        Disk_raid_type_text = new javax.swing.JLabel();
        jPanel4 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();

        setMaximumSize(new java.awt.Dimension(640, 480));
        setMinimumSize(new java.awt.Dimension(640, 480));
        setPreferredSize(new java.awt.Dimension(640, 480));

        jScrollPane5.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane5.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane5.setMaximumSize(new java.awt.Dimension(400, 300));
        jScrollPane5.setMinimumSize(new java.awt.Dimension(400, 300));
        jScrollPane5.setPreferredSize(new java.awt.Dimension(400, 300));

        Setup_details.setBackground(new java.awt.Color(220, 234, 247));
        Setup_details.setMaximumSize(new java.awt.Dimension(400, 300));
        Setup_details.setMinimumSize(new java.awt.Dimension(400, 300));
        Setup_details.setPreferredSize(new java.awt.Dimension(400, 300));

        jPanel1.setBackground(new java.awt.Color(220, 234, 247));

        Disk_total_num.setFont(new java.awt.Font("新細明體", 1, 12));
        Disk_total_num.setText("Disk Total Number:");

        Disk_total_num_text.setText("        ");

        Disk_block_size.setFont(new java.awt.Font("新細明體", 1, 12));
        Disk_block_size.setText("Disk Block Size:");

        Disk_block_size_text.setText("        ");

        Disk_raid_type.setFont(new java.awt.Font("新細明體", 1, 12));
        Disk_raid_type.setText("Disk Raid Type:");

        Disk_raid_type_text.setText("        ");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(Disk_total_num)
                    .addComponent(Disk_block_size)
                    .addComponent(Disk_raid_type))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(Disk_block_size_text, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(Disk_total_num_text, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(Disk_raid_type_text))
                .addContainerGap(55, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Disk_total_num)
                    .addComponent(Disk_total_num_text))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Disk_block_size, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Disk_block_size_text))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(Disk_raid_type, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(Disk_raid_type_text))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel4.setBackground(new java.awt.Color(220, 234, 247));

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 199, Short.MAX_VALUE)
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 109, Short.MAX_VALUE)
        );

        jLabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/ECFS_setup3.png"))); // NOI18N

        javax.swing.GroupLayout Setup_detailsLayout = new javax.swing.GroupLayout(Setup_details);
        Setup_details.setLayout(Setup_detailsLayout);
        Setup_detailsLayout.setHorizontalGroup(
            Setup_detailsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(Setup_detailsLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addGroup(Setup_detailsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(Setup_detailsLayout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(Setup_detailsLayout.createSequentialGroup()
                        .addGap(14, 14, 14)
                        .addComponent(jPanel4, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        Setup_detailsLayout.setVerticalGroup(
            Setup_detailsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(Setup_detailsLayout.createSequentialGroup()
                .addGroup(Setup_detailsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(Setup_detailsLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(jLabel1))
                .addContainerGap(165, Short.MAX_VALUE))
        );

        jScrollPane5.setViewportView(Setup_details);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jScrollPane5, javax.swing.GroupLayout.DEFAULT_SIZE, 630, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jScrollPane5, javax.swing.GroupLayout.DEFAULT_SIZE, 469, Short.MAX_VALUE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    String Raid = "";
    JLabel[] Disk;
    public long global_disk_size;
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel Disk_block_size;
    private javax.swing.JLabel Disk_block_size_text;
    private javax.swing.JLabel Disk_raid_type;
    private javax.swing.JLabel Disk_raid_type_text;
    private javax.swing.JLabel Disk_total_num;
    private javax.swing.JLabel Disk_total_num_text;
    private javax.swing.JPanel Setup_details;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JScrollPane jScrollPane5;
    // End of variables declaration//GEN-END:variables
}
