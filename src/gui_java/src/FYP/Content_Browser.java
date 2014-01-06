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

import java.awt.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Timer;
import javax.swing.GroupLayout;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import java.util.TimerTask;

/**
 *
 * @author user
 */
public class Content_Browser extends javax.swing.JPanel {

    /** Creates new form Setup_Details */
    public Content_Browser() {
        initComponents();

        Action.setText(null);
        ElapsedTime.setText(null);
        Throughput.setText(null);
        DataSize.setText(null);

        if (Global.testing)
        {
            Global.sd.SendByte((byte)3);
            Global.sd.SendByte((byte)0);
            Global.sd.SendString(Previous_dir);
        }
        Location.setText(Previous_dir);
        //SwingUtilities.invokeLater(new Start(str));
    }

    public void ProcessHandler() throws InterruptedException{
        //Stage instage;
        //boolean working;
        byte inbyte;
        //int inInt;
        //byte inbyte2;
        String str;
        byte[] byte_str;
        //byte[] byte_str = new byte[1024];

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
                    //System.out.println(temp_str.length);
                    temp_str = new String[temp_str.length + 1];
                    //System.out.println(temp_str.length);
                    System.arraycopy(temp, 0, temp_str, 0, temp_str.length - 1);
                    //System.out.println(temp_str.length);
                    temp_str[temp_str.length-1] = str;
                }
                else SwingUtilities.invokeLater(new Set_Content(temp_str));
        }
        /*else if (inbyte == 1)   //upload
        {
        }*/
        else if (inbyte == 2)   //download
        {
            try{
                if (Global_file_length == -1){
                    Global_file_length = Global.sd.ReadInt();
                    Total_length = Global_file_length;
                    //System.out.println("Files size " + Global_file_length);
                }
                else
                {
                    if (Global_file_length > 1024)
                    {
                        byte_str = new byte[1024];
                        Global.sd.ReadFully(byte_str, 0, 1024);
                        fstream_w.write(byte_str);
                        //str = Global.sd.ReadString();
                        //Global_out.write(str);
                        Global_file_length = Global_file_length - 1024;
                        Global.ProgressBar.setNumber(Total_length - Global_file_length,
                                (int)((Total_length - Global_file_length)*100/Total_length));
                    }
                    else
                    {
                        if(Global_file_length > 0){
                        byte_str = new byte[(int) Global_file_length];
                        Global.sd.ReadFully(byte_str, 0, (int) Global_file_length);
                        fstream_w.write(byte_str);
                        }
                        //fstream_w.write(byte_str, 0, Global_file_length);
                        Global_file_length = -1;
                        fstream_w.close();
                        //Global_out.close();
                        Global.ProgressBar.setNumber(Total_length, 100);
                        Global.ProgressBar.setCancel();
                        doing_job = false;
                        job2 = false;
                        File_Back[chosen].setBackground(null);
                    }
                }
            } catch (IOException e) {
                return;
            }
        }
        //else return;
    }

     /**
     * @brief   Update the GUI after the event-dispatching thread
     */
    private class Set_Content implements Runnable{
        private String[] str;
        Set_Content(String[] _str){
            str = _str;
        }
        public void run() {
            setLabel(str.length, str);
        }
    }

    public void setLabel(int no_of_file, String[] str){
         File_Back = new javax.swing.JPanel[no_of_file];
         Files = new javax.swing.JLabel[no_of_file];
         Full_Path = new String[no_of_file];
         Check_Dir_Type = new boolean[no_of_file];
         Global_no_of_file = no_of_file;

         chosen = 0;
         choose_click = false;
         //cut = false;
         job2 = false;

         Location.setText(Previous_dir);

        //Copy_file.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Copy_24x24.png"))); // NOI18N

        for (int i = 0; i < no_of_file; i++)
        {
            Full_Path[i] = Previous_dir + str[i];

            Files[i] = new javax.swing.JLabel();
            if (check_directory(str[i]))
            {
                Check_Dir_Type[i] = true;
                Files[i].setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Folder_48x48.png")));
            }
            else
            {
                Check_Dir_Type[i] = false;
                Files[i].setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/New_48x48.png")));
            }
            //Disk[i].setText(str[i]);
            //Files[i].setText(file_name(str[i]));
            Files[i].setText(str[i]);
            Files[i].setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
            Files[i].setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);

            //if (Check_Dir_Type[i])
            //{
                Files[i].addMouseListener(new java.awt.event.MouseAdapter() {
                    @Override
                    public void mouseClicked(java.awt.event.MouseEvent evt) {
                        if (evt.getClickCount() == 2)
                        {
                            //System.out.println(evt.getSource().toString());
                            int i;
                            for (i = 0; i < Global_no_of_file; i++)
                                if (Files[i] == evt.getComponent())
                                {
                                    //System.out.println(Full_Path[i]);
                                    Previous_dir = Full_Path[i];

                                    //System.out.println(Previous_dir);
                                    Global.lock.lock();
                                    try{
                                        Global.sd.SendByte((byte)3);
                                        Global.sd.SendByte((byte)0);
                                        Global.sd.SendString(Previous_dir);
                                    }
                                    finally {
                                        Global.lock.unlock();
                                    }
                                    break;
                                }
                        }
                    }
                });
            //}
            //else
            //{
                Files[i].addMouseListener(new java.awt.event.MouseAdapter() {
                    @Override
                    public void mouseClicked(java.awt.event.MouseEvent evt) {
                        evt2 = evt;
                            for (int i = 0; i < Global_no_of_file; i++)
                                if (Files[i] == evt.getComponent())
                                {
                                    File_Back[chosen].setBackground(null);
                                    File_Back[i].setBackground(Color.lightGray);
                                    chosen = i;
                                    choose_click = true;
                                    break;
                                }
                            job2 = true;
                    }
                });
            //}

            File_Back[i] = new javax.swing.JPanel();
            javax.swing.GroupLayout File_BackLayout = new javax.swing.GroupLayout(File_Back[i]);
            File_Back[i].setLayout(File_BackLayout);
            File_BackLayout.setHorizontalGroup(
                File_BackLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(File_BackLayout.createSequentialGroup()
                    .addContainerGap()
                    //.addGroup(File_BackLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(Files[i])
                    .addContainerGap())
            );
            File_BackLayout.setVerticalGroup(
                File_BackLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(File_BackLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(Files[i])
                    .addContainerGap())
            );
        }

        int no_of_row = no_of_file/6;
        if (no_of_file % 6 != 0)
            no_of_row++;

        jPanel1 = new javax.swing.JPanel();
        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);

        GroupLayout.ParallelGroup PG = jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING);
        GroupLayout.SequentialGroup[] SG = new GroupLayout.SequentialGroup[no_of_row];
        for (int j = 0; j < no_of_row; j++)
        {
             SG[j] = jPanel1Layout.createSequentialGroup();
             PG.addGroup(SG[j]);
        }
        int count = -1;
        for (int i = 0; i < no_of_file; i++)
        {
            if (i % 6 == 0)
            {
                count++;
                SG[count].addGap(15);
            }
            SG[count].addComponent(File_Back[i]);
            SG[count].addGap(10);
        }
        jPanel1Layout.setHorizontalGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING).addGroup(PG));

        GroupLayout.SequentialGroup SGV = jPanel1Layout.createSequentialGroup();
        GroupLayout.ParallelGroup[] PGV = new GroupLayout.ParallelGroup[no_of_row];
        for (int j = 0; j < no_of_row; j++)
        {
             PGV[j] = jPanel1Layout.createParallelGroup();
             SGV.addGap(10);
             SGV.addGroup(PGV[j]);
        }
        count = -1;
        for (int i = 0; i < no_of_file; i++)
        {
            if (i % 6 == 0)
            {
                count++;
                PGV[count].addGap(10);
            }
            PGV[count].addComponent(File_Back[i]);
            PGV[count].addGap(10);
        }
        jPanel1Layout.setVerticalGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING).addGroup(SGV));

        jScrollPane1.setViewportView(jPanel1);
        temp_str = new String[0];
    }

/*public static boolean is_param(String line, String param_str){
    return line.startsWith(param_str);
    //return param_str.substring(0, param_str.length() - 1).compareTo(line.substring(0, param_str.length() - 1));
}

public static String global_param_str(String line, String param_str){
    if (is_param((line), (param_str))){
        return line.substring(param_str.length() + 1);
        //String temp = line.substring(param_str.length() + 1);
        //return Integer.parseInt(temp);
    }
    return null;
}*/

public static String file_name(String line){
    int i;
    int length = line.length();
    for (i = length-2; i >= 0; i--)
    {
        if (line.charAt(i) == '/')
            break;
    }
    /*if (line.charAt(length-1) == '/')
        return line.substring(i+1, length-1);
    else return line.substring(i+1, length);*/
    return line.substring(i+1, length);
}

public static String file_name_2(String line){      //can be used to check on windows and linux
    int i;
    int length = line.length();
    for (i = length-2; i >= 0; i--)
    {
        if ((line.charAt(i) == '/') || (line.charAt(i) == '\\'))
            break;
    }
    if ((line.charAt(length-1) == '/') || (line.charAt(length-1) == '\\'))
        return line.substring(i+1, length-1);
    else return line.substring(i+1, length);
}

public static String get_current_dir(String line){
    int i;
    int length = line.length();
    for (i = length-2; i >= 0; i--)
    {
        if (line.charAt(i) == '/')
            break;
    }
    if (i == 0)
        return "/";
    else return line.substring(0, i+1);
}

public static boolean check_directory(String line){
    return (line.charAt(line.length()-1) == '/');
}
/*public static disk_param_string(line, param, param_str)
    do {
        char *dot = strchr(line, '.');
        if (dot && is_param(dot + 1, (param_str))) {
            char *val = dot + 1 + sizeof(param_str);
            size_t len = strlen(val);
            char *dev = (char *)calloc(len, sizeof(char));
            strncpy(dev, val, len);
            param = dev;
        }
    } while(0)*/

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        PreviousButton = new javax.swing.JButton();
        RefreshButton = new javax.swing.JButton();
        Action1 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        Action = new javax.swing.JLabel();
        Location = new javax.swing.JLabel();
        toolBar = new javax.swing.JToolBar();
        UploadButton = new javax.swing.JButton();
        DownloadButton = new javax.swing.JButton();
        CutButton = new javax.swing.JButton();
        CopyButton = new javax.swing.JButton();
        PasteButton = new javax.swing.JButton();
        DeleteButton = new javax.swing.JButton();
        RenameButton = new javax.swing.JButton();
        NewFolderButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel1 = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        toolBar1 = new javax.swing.JToolBar();
        PreviousButton1 = new javax.swing.JButton();
        RefreshButton1 = new javax.swing.JButton();
        ElapsedTime = new javax.swing.JLabel();
        Throughput = new javax.swing.JLabel();
        DataSize = new javax.swing.JLabel();

        PreviousButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Previous_24x24.png"))); // NOI18N
        PreviousButton.setText("Previous");
        PreviousButton.setFocusable(false);
        PreviousButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PreviousButtonActionPerformed(evt);
            }
        });

        RefreshButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Refresh_24x24.png"))); // NOI18N
        RefreshButton.setText("Refresh");
        RefreshButton.setFocusable(false);
        RefreshButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RefreshButtonActionPerformed(evt);
            }
        });

        Action1.setText("Operation");

        setBackground(new java.awt.Color(220, 234, 247));
        setMaximumSize(new java.awt.Dimension(640, 400));
        setMinimumSize(new java.awt.Dimension(640, 400));

        jPanel2.setBackground(new java.awt.Color(220, 234, 247));

        Action.setText("Operation");

        Location.setText("/");

        toolBar.setFloatable(false);
        toolBar.setRollover(true);

        UploadButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Upload_24x24.png"))); // NOI18N
        UploadButton.setText("Upload");
        UploadButton.setFocusable(false);
        UploadButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        UploadButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UploadButtonActionPerformed(evt);
            }
        });
        toolBar.add(UploadButton);

        DownloadButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Download_24x24.png"))); // NOI18N
        DownloadButton.setText("Download");
        DownloadButton.setFocusable(false);
        DownloadButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        DownloadButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DownloadButtonActionPerformed(evt);
            }
        });
        toolBar.add(DownloadButton);

        CutButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Cut_24x24.png"))); // NOI18N
        CutButton.setText("Cut");
        CutButton.setFocusable(false);
        CutButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        CutButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CutButtonActionPerformed(evt);
            }
        });
        toolBar.add(CutButton);

        CopyButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Copy_24x24.png"))); // NOI18N
        CopyButton.setText("Copy");
        CopyButton.setFocusable(false);
        CopyButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        CopyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CopyButtonActionPerformed(evt);
            }
        });
        toolBar.add(CopyButton);

        PasteButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Paste_24x24.png"))); // NOI18N
        PasteButton.setText("Paste");
        PasteButton.setFocusable(false);
        PasteButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        PasteButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PasteButtonActionPerformed(evt);
            }
        });
        toolBar.add(PasteButton);

        DeleteButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Delete_24x24.png"))); // NOI18N
        DeleteButton.setText("Delete");
        DeleteButton.setFocusable(false);
        DeleteButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        DeleteButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DeleteButtonActionPerformed(evt);
            }
        });
        toolBar.add(DeleteButton);

        RenameButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Rename_24x24.png"))); // NOI18N
        RenameButton.setText("Rename");
        RenameButton.setFocusable(false);
        RenameButton.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        RenameButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RenameButtonActionPerformed(evt);
            }
        });
        toolBar.add(RenameButton);

        NewFolderButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Folder_24x24.png"))); // NOI18N
        NewFolderButton.setText("New Folder");
        NewFolderButton.setFocusable(false);
        NewFolderButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NewFolderButtonActionPerformed(evt);
            }
        });
        toolBar.add(NewFolderButton);

        jLabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Content_Browser.png"))); // NOI18N

        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setMaximumSize(new java.awt.Dimension(600, 360));
        jScrollPane1.setMinimumSize(new java.awt.Dimension(600, 360));
        jScrollPane1.setPreferredSize(new java.awt.Dimension(600, 360));

        jPanel1.setMaximumSize(new java.awt.Dimension(600, 250));
        jPanel1.setMinimumSize(new java.awt.Dimension(600, 250));
        jPanel1.setPreferredSize(new java.awt.Dimension(600, 250));
        jPanel1.setRequestFocusEnabled(false);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 600, Short.MAX_VALUE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 250, Short.MAX_VALUE)
        );

        jScrollPane1.setViewportView(jPanel1);

        jLabel2.setText("Location:");

        toolBar1.setFloatable(false);
        toolBar1.setRollover(true);

        PreviousButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Previous_24x24.png"))); // NOI18N
        PreviousButton1.setText("Previous");
        PreviousButton1.setFocusable(false);
        PreviousButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PreviousButton1ActionPerformed(evt);
            }
        });
        toolBar1.add(PreviousButton1);

        RefreshButton1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Refresh_24x24.png"))); // NOI18N
        RefreshButton1.setText("Refresh");
        RefreshButton1.setFocusable(false);
        RefreshButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RefreshButton1ActionPerformed(evt);
            }
        });
        toolBar1.add(RefreshButton1);

        ElapsedTime.setText("Elapsed Time: ");

        Throughput.setText("Throughput: ");

        DataSize.setText("Data size: ");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 618, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(toolBar1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addComponent(jLabel2)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(Location, javax.swing.GroupLayout.PREFERRED_SIZE, 315, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(Action, javax.swing.GroupLayout.PREFERRED_SIZE, 263, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 75, Short.MAX_VALUE)
                        .addComponent(jLabel1))
                    .addComponent(toolBar, javax.swing.GroupLayout.PREFERRED_SIZE, 618, Short.MAX_VALUE)
                    .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(ElapsedTime, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, 263, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(Throughput, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, 263, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addComponent(DataSize, javax.swing.GroupLayout.PREFERRED_SIZE, 263, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                .addComponent(toolBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(10, 10, 10)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 224, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(42, 42, 42)
                        .addComponent(jLabel1))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(18, 18, 18)
                        .addComponent(toolBar1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(Location))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(Action, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addGap(1, 1, 1)
                .addComponent(ElapsedTime, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(Throughput, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(DataSize, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
    }// </editor-fold>//GEN-END:initComponents

    private void UploadButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UploadButtonActionPerformed
        // TODO add your handling code here:
        //new Upload_File_Chooser(this,true).setVisible(true);
        if (!doing_job)
        {
            JFileChooser jFileChooser1 = new JFileChooser();
            jFileChooser1.setDialogTitle("Upload Files from");        //Upload only one Files
            //jFileChooser1.setCurrentDirectory(new java.io.Files("C:\\Users\\Andrew Kong\\Desktop"));
            jFileChooser1.setFileSelectionMode(javax.swing.JFileChooser.FILES_AND_DIRECTORIES);
            jFileChooser1.setApproveButtonText("Upload");
            if (Upload_dir_file != null)
                jFileChooser1.setSelectedFile(Upload_dir_file);
            int Option = jFileChooser1.showOpenDialog(this);
            if (Option == JFileChooser.APPROVE_OPTION) {
                doing_job = true;
                File filePath = jFileChooser1.getSelectedFile();
                System.out.println(filePath);
                Upload_dir_file = filePath;
                Global.ProgressBar = new Progress_Bar(Global.screenSize, Global.frameSize);
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                Global.ProgressBar.setLabel(filePath.toString(), "Upload to:", Previous_dir+file_name_2(filePath.toString()));
                Global.ProgressBar.setTitle("Uploading...");
                Global.ProgressBar.setVisible(true);
                
                /////// /root/ABC/a.c +'\\'
                //read Files
                try{
                    fstream_r = new FileInputStream(filePath.toString());
                    ////////////////////////////
                    Global_file_length = filePath.length();
                    Total_length = Global_file_length;
                    System.out.println(Global_file_length);

                    Global.lock.lock();
                    try{
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)1);
                    Global.sd.SendString(Previous_dir+file_name_2(filePath.toString()));
                    Global.sd.SendLong(Global_file_length);

                    }
                    finally {
                        Global.lock.unlock();
                    }

                    //in = new DataInputStream(fstream_r);
                    //Global_in = new BufferedReader(new InputStreamReader(in));

                    Timer timer = new Timer();
                    TimerTask task = new MyTask();
                    timer.schedule(task, 0, 1);
                    //char[] char_buf = new char[1024];
                    /*char[] char_buf;

                    while (Global_file_length > 0)
                    {
                        if (Global_file_length >= 1024)
                        {
                            char_buf = new char[1024];
                            Global_file_length = Global_file_length - 1024;
                        }
                        else
                        {
                            char_buf = new char[(int) Global_file_length];
                            Global_file_length = 0;
                        }
                        int unuseful = Global_in.read(char_buf);
                        //Global_in.read(char_buf);
                        buffer = new String(char_buf);
                        System.out.println(buffer);
                        Global.sd.SendString(buffer);
                    }
                    fstream_r.close();
                    in.close();
                    Global_in.close();*/
                }catch (Exception e){//Catch exception if any
                    System.err.println("Error: " + e.getMessage());
                }
                //Global_filename = filePath.toString();

                From_file=file_name_2(filePath.toString());
                Action.setText("Upload "+file_name(From_file));
                ElapsedTime.setText("Elapsed Time: 123");
                Throughput.setText("Throughput: 567");

            } else if (Option == JFileChooser.CANCEL_OPTION) {
                jFileChooser1.setVisible(false);
            }
            //System.out.println(JFileChooser.APPROVE_OPTION);
            //System.out.println(JFileChooser.CANCEL_OPTION);
        }
}//GEN-LAST:event_UploadButtonActionPerformed

    public class MyTask extends java.util.TimerTask{
    @Override
        public void run() {
            //char[] char_buf;
            byte[] byte_buf;
            //byte[] byte_buf = new byte[1024];

            try{
                if (Global_file_length >= 40960)
                {
                    //char_buf = new char[1024];
                    byte_buf = new byte[40960];
                    //Global_file_length = Global_file_length - 1024;
                    int unuseful = fstream_r.read(byte_buf);
                    //int unuseful = Global_in.read(char_buf);
                    //buffer = new String(char_buf);
                    //buffer = new String(byte_buf);

                    for (int k = 0; k < 40960; k = k + 1024)
                    {
                        Global_file_length = Global_file_length - 1024;
                    Global.lock.lock();
                    try{
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)1);
                    Global.sd.SendFully(byte_buf, k, 1024);
                    }
                    finally {
                        Global.lock.unlock();
                    }
                    Global.ProgressBar.setNumber(Total_length - Global_file_length,
                            (int)((Total_length - Global_file_length)*100/Total_length));
                    }
                }
                else
                {
                    //char_buf = new char[(int) Global_file_length];
                    if(Global_file_length > 0){
                    byte_buf = new byte[(int) Global_file_length];
                    //int unuseful = Global_in.read(char_buf);
                    //buffer = new String(char_buf);
                    int unuseful = fstream_r.read(byte_buf);
                    //int unuseful = fstream_r.read(byte_buf, 0, (int) Global_file_length);

                    int h = 0;
                    while (Global_file_length > 1024)
                    {
                        Global_file_length = Global_file_length - 1024;
                    Global.lock.lock();
                    try{
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)1);
                    Global.sd.SendFully(byte_buf, h, 1024);
                    }
                    finally {
                        Global.lock.unlock();
                    }
                    Global.ProgressBar.setNumber(Total_length - Global_file_length,
                            (int)((Total_length - Global_file_length)*100/Total_length));
                    h = h + 1024;
                    }

                    Global.lock.lock();
                    try{
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)1);
                    Global.sd.SendFully(byte_buf, h, (int) Global_file_length);
                    }
                    finally {
                        Global.lock.unlock();
                    }
                    }
                    Global_file_length = -1;
                    fstream_r.close();
                    //in.close();
                    //Global_in.close();

                    Global.ProgressBar.setNumber(Total_length, 100);

                    Global.lock.lock();
                    try {
                        Global.sd.SendByte((byte) 3);
                        Global.sd.SendByte((byte) 0);
                        Global.sd.SendString(Previous_dir);
                    } finally {
                        Global.lock.unlock();
                    }

                    Global.ProgressBar.setCancel();
                    doing_job = false;
                    //Global.ProgressBar.setLabel(null, null, null);
                    this.cancel();
                }
            }catch (Exception e){//Catch exception if any
                System.err.println("Error: " + e.getMessage());
            }
            /*while (Global_file_length > 0)
            {
                if (Global_file_length >= 1024)
                {
                    char_buf = new char[1024];
                    Global_file_length = Global_file_length - 1024;
                }
                else
                {
                    char_buf = new char[(int) Global_file_length];
                    Global_file_length = 0;
                }
                int unuseful = Global_in.read(char_buf);
                //Global_in.read(char_buf);
                buffer = new String(char_buf);
                System.out.println(buffer);
                Global.sd.SendString(buffer);
            }
            fstream_r.close();
            in.close();
            Global_in.close();*/
        }
    }

    private void DownloadButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DownloadButtonActionPerformed
        // TODO add your handling code here:
      if (!Check_Dir_Type[chosen] && choose_click)
        if (job2 && !doing_job)
        {
            JFileChooser jFileChooser1 = new JFileChooser();
            jFileChooser1.setDialogTitle("Download Files to");       //Downlaod only one Files
            //jFileChooser1.setCurrentDirectory(new java.io.Files("C:\\Users\\Andrew Kong\\Desktop"));
            jFileChooser1.setDialogType(javax.swing.JFileChooser.SAVE_DIALOG);
            jFileChooser1.setFileSelectionMode(javax.swing.JFileChooser.FILES_AND_DIRECTORIES);
            jFileChooser1.setApproveButtonText("Save");
            if (Download_dir != null)
                jFileChooser1.setSelectedFile(new File(Download_dir+file_name(Full_Path[chosen])));
            else jFileChooser1.setSelectedFile(new File(file_name(Full_Path[chosen])));
            //jFileChooser1.setSelectedFile(new java.io.Files("C:\\Users\\Andrew Kong\\Desktop\\FYP_GUI.java"));
            int Option = jFileChooser1.showSaveDialog(this);
            if (Option == JFileChooser.APPROVE_OPTION) {
                doing_job = true;
                File filePath = jFileChooser1.getSelectedFile();
                System.out.println(filePath);
                //Download_dir = get_current_dir(filePath.toString());
                if(filePath.isDirectory())
                    filePath = new File(filePath.toString() + File.separator + file_name(Full_Path[chosen]));
                job2 = false;
                Global.ProgressBar = new Progress_Bar(Global.screenSize, Global.frameSize);
                Global.ProgressBar.setLabel(Full_Path[chosen], "Download to:", filePath.toString());
                Global.ProgressBar.setTitle("Downloading...");
                Global.ProgressBar.setVisible(true);

                //write Files
                try{
                    //fstream_w = new FileWriter(filePath.toString());
                    fstream_w = new FileOutputStream(filePath.toString());
                    //Global_out = new BufferedWriter(fstream_w);

                    //Global.lock.lock();
                    //try{
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)2);
                    Global.sd.SendString(Full_Path[chosen]);
                    /*}
                    finally {
                        Global.lock.unlock();
                    }*/

                }catch (Exception e){//Catch exception if any
                    System.err.println("Error: " + e.getMessage());
                }
                //Global_filename = filePath.toString();
                From_file = Full_Path[chosen];
                Action.setText("Download "+file_name(From_file));
                ElapsedTime.setText("Elapsed Time: B123");
                Throughput.setText("Throughput: B567");

            } else if (Option == JFileChooser.CANCEL_OPTION) {
                jFileChooser1.setVisible(false);
            }

            //System.out.println(JFileChooser.APPROVE_OPTION);
            //System.out.println(JFileChooser.CANCEL_OPTION);
        }
}//GEN-LAST:event_DownloadButtonActionPerformed

    private void CutButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CutButtonActionPerformed
        // TODO add your handling code here:
        if (choose_click)
        {
            cut = true;
            From_file = Full_Path[chosen];
            Action.setText("Cut "+file_name(From_file));
        }
}//GEN-LAST:event_CutButtonActionPerformed

    private void CopyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CopyButtonActionPerformed
        // TODO add your handling code here:
        if (choose_click)
        {
            cut = false;
            From_file = Full_Path[chosen];
            Action.setText("Copy "+file_name(From_file));
        }
}//GEN-LAST:event_CopyButtonActionPerformed

    private void PasteButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PasteButtonActionPerformed
        // TODO add your handling code here:
        if (From_file != null)
        {
            Global.lock.lock();
            try {
                if(cut == false){
                Action.setText("Paste "+file_name(From_file));
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendString(From_file);
                    Global.sd.SendString(Previous_dir+file_name(From_file));
                } else {
                        Action.setText("Move "+From_file+" to "+file_name(From_file));
                        Global.sd.SendByte((byte)3);
                        Global.sd.SendByte((byte)5);
                        Global.sd.SendString(From_file);
                        Global.sd.SendString(Previous_dir+file_name(From_file));
                }
            } finally {
                Global.lock.unlock();
            }
            From_file = null;
            cut = false;
            //choose_click = false;
            RefreshContent();
        }
}//GEN-LAST:event_PasteButtonActionPerformed

    private void DeleteButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DeleteButtonActionPerformed
        // TODO add your handling code here:
        if (choose_click)
        {
            Global.lock.lock();
                try {
                    Action.setText("Delete "+file_name(Full_Path[chosen]));
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)4);
                    Global.sd.SendString(Full_Path[chosen]);
                } finally {
                    Global.lock.unlock();
            }
            From_file = null;
            choose_click = false;
            RefreshContent();
        }
}//GEN-LAST:event_DeleteButtonActionPerformed

    private void RenameButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RenameButtonActionPerformed
        // TODO add your handling code here:
        if (choose_click)
        {
            Global.RenameText.setText(file_name(Full_Path[chosen]));
            //Global.RenameText.setLocation(Files[chosen].getX(), Files[chosen].getY());
            Global.RenameText.setLocation(evt2.getXOnScreen(), evt2.getYOnScreen());
            Global.RenameText.setVisible(true);
            //System.out.println(Previous_dir + Global.Rename_name);
            if (Check_Dir_Type[chosen] == check_directory(Global.Rename_name))
            {
                Global.lock.lock();
                    try {
                        Action.setText("Rename "+file_name(Full_Path[chosen]));
                        Global.sd.SendByte((byte)3);
                        Global.sd.SendByte((byte)5);
                        Global.sd.SendString(Full_Path[chosen]);
                        Global.sd.SendString(Previous_dir + Global.Rename_name);
                        //System.out.println(Previous_dir + Global.Rename_name);
                    } finally {
                        Global.lock.unlock();
                    }
                From_file = null;
                RefreshContent();
            }
        }
}//GEN-LAST:event_RenameButtonActionPerformed

    private void PreviousButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PreviousButtonActionPerformed
        if (!Previous_dir.equals("/"))
            Previous_dir = get_current_dir(Previous_dir);
        System.out.println(Previous_dir);
        Global.lock.lock();
        try {
            Global.sd.SendByte((byte) 3);
            Global.sd.SendByte((byte) 0);
            Global.sd.SendString(Previous_dir);
        } finally {
            Global.lock.unlock();
        }
    }//GEN-LAST:event_PreviousButtonActionPerformed

    public void RefreshContent(){
        Global.lock.lock();
        try {
            Global.sd.SendByte((byte) 3);
            Global.sd.SendByte((byte) 0);
            Global.sd.SendString(Previous_dir);
        } finally {
            Global.lock.unlock();
        }
    }

    private void RefreshButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RefreshButtonActionPerformed
        RefreshContent();
    }//GEN-LAST:event_RefreshButtonActionPerformed

    private void PreviousButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PreviousButton1ActionPerformed
        // TODO add your handling code here:
        if (!Previous_dir.equals("/"))
            Previous_dir = get_current_dir(Previous_dir);
        System.out.println(Previous_dir);
        Global.lock.lock();
        try {
            Global.sd.SendByte((byte) 3);
            Global.sd.SendByte((byte) 0);
            Global.sd.SendString(Previous_dir);
        } finally {
            Global.lock.unlock();
        }
    }//GEN-LAST:event_PreviousButton1ActionPerformed

    private void RefreshButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RefreshButton1ActionPerformed
        // TODO add your handling code here:
        RefreshContent();
    }//GEN-LAST:event_RefreshButton1ActionPerformed

    private void NewFolderButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NewFolderButtonActionPerformed
        // TODO add your handling code here:
        Global.RenameText.setTitle("Create");
        Global.RenameText.setText("New Folder/");
        Global.RenameText.setLocation(Global.screenSize.width/2, Global.screenSize.height/2);
        Global.RenameText.setVisible(true);
        if (check_directory(Global.Rename_name))
        {
            Global.lock.lock();
                try {
                    Action.setText("Create "+Global.Rename_name);
                    Global.sd.SendByte((byte)3);
                    Global.sd.SendByte((byte)6);
                    Global.sd.SendString(Previous_dir + Global.Rename_name);
                    //System.out.println(Previous_dir + Global.Rename_name);
                } finally {
                    Global.lock.unlock();
                }
            RefreshContent();
        }
        Global.RenameText.setTitle(null);
    }//GEN-LAST:event_NewFolderButtonActionPerformed

    //private Label[] label;
    //boolean job1 = true;
    boolean job2 = true;
    boolean doing_job = false;
    JPanel[] File_Back;
    JLabel[] Files;
    String[] Full_Path;
    boolean[] Check_Dir_Type;
    int Global_no_of_file;
    int chosen;
    boolean choose_click = false;
    String Previous_dir = "/";
    String[] temp_str = {};

    //String Global_filename = null;
    File Upload_dir_file = null;
    FileInputStream fstream_r;
    //FileWriter fstream_w;
    String Download_dir = null;
    FileOutputStream fstream_w;
    //BufferedWriter Global_out;
    //DataInputStream in;
    //BufferedReader Global_in;
    long Global_file_length = -1;
    //int Global_offset = 0;
    //String buffer = null;

    long Total_length = 0;

    String From_file = null;
    boolean cut = false;
    java.awt.event.MouseEvent evt2;
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel Action;
    private javax.swing.JLabel Action1;
    private javax.swing.JButton CopyButton;
    private javax.swing.JButton CutButton;
    public javax.swing.JLabel DataSize;
    private javax.swing.JButton DeleteButton;
    private javax.swing.JButton DownloadButton;
    public javax.swing.JLabel ElapsedTime;
    private javax.swing.JLabel Location;
    private javax.swing.JButton NewFolderButton;
    private javax.swing.JButton PasteButton;
    private javax.swing.JButton PreviousButton;
    private javax.swing.JButton PreviousButton1;
    private javax.swing.JButton RefreshButton;
    private javax.swing.JButton RefreshButton1;
    private javax.swing.JButton RenameButton;
    public javax.swing.JLabel Throughput;
    private javax.swing.JButton UploadButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JToolBar toolBar;
    private javax.swing.JToolBar toolBar1;
    // End of variables declaration//GEN-END:variables
}
