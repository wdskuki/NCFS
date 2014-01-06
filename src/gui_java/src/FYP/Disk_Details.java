/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * Disk_Details_Child.java
 *
 * Created on 2010/10/24, 下午 08:16:05
 */

package FYP;

import java.io.IOException;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.ParallelGroup;
import javax.swing.GroupLayout.SequentialGroup;
import javax.swing.SwingUtilities;

/**
 *
 * @author user
 */
public class Disk_Details extends javax.swing.JPanel {
    private Disk_Details_Child[] Child;
    private Disk_Details_Overall_Child[] Overall_Child;
    private int numofchild;
    private GroupLayout jPanel4Layout;
    private ParallelGroup RowGroup[];
    private SequentialGroup Col;
    private ParallelGroup ColGroup[];

    /** Creates new form Disk_Details */
    public Disk_Details() {
        Child = new Disk_Details_Child[100];
        Overall_Child = new Disk_Details_Overall_Child[100];
        numofchild = 0;
        initComponents();
        jPanel4Layout = new GroupLayout(Overall_Panel);
        Overall_Panel.setLayout(jPanel4Layout);

        RowGroup = new ParallelGroup[2];
        ColGroup = new ParallelGroup[50];
        
        for(int i = 0; i < 2; ++i)
            RowGroup[i] = jPanel4Layout.createParallelGroup(GroupLayout.Alignment.LEADING);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createSequentialGroup()
                .addGroup(RowGroup[0])
                .addGroup(RowGroup[1])
            );
        Col = jPanel4Layout.createSequentialGroup();
        jPanel4Layout.setVerticalGroup(Col);

    }


    /**
     * @author  windkit
     * @brief   Request the server to update a disk usage bitmap
     */
    public void request_Update(int diskid){
        Global.sd.SendByte((byte)1);
        Global.sd.SendInt(diskid);
    }

    public void add_Tab(String tab_title, Disk_Details_Child DD){
        //System.out.println("Child Num1 " + numofchild);
        Disk_Details_Tab.addTab(tab_title, DD);
        Child[numofchild] = DD;
        //request_Update(numofchild);
        Disk_Details_Overall_Child DDO = new Disk_Details_Overall_Child();
        add_Overall_Child("Disk " + numofchild,DDO);

        int row = numofchild % 2;
        int col = numofchild / 2;

        if((row) == 0){
            ColGroup[col] = jPanel4Layout.createParallelGroup(GroupLayout.Alignment.LEADING);
            Col.addGroup(ColGroup[col]);
        }
        RowGroup[row].addComponent(DDO);
        ColGroup[col].addComponent(DDO);
        request_Update(numofchild);
        numofchild++;


        //System.out.println("Child Num " + numofchild);
    }

    public void add_Overall_Child(String tab_title, Disk_Details_Overall_Child DDO){
        //System.out.println("Child Num1 " + numofchild);
        //Disk_Details_Tab.addTab(tab_title, DD);
        Overall_Child[numofchild] = DDO;
        DDO.setTitle(tab_title);
        //Overall_Panel.add(tab_title, DD);
        //System.out.println("Child Num " + numofchild);
    }

    /*
     * Depreciated 7-12-2010
     */
    public void complete_Overall(){
        //System.out.println("Child Num1 " + numofchild);
        int no_of_row = numofchild/2;
        if (numofchild % 2 != 0)
            no_of_row++;



        GroupLayout.ParallelGroup PG = jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING);
        GroupLayout.SequentialGroup[] SG = new GroupLayout.SequentialGroup[no_of_row];
        for (int j = 0; j < no_of_row; j++)
        {
             SG[j] = jPanel4Layout.createSequentialGroup();
             PG.addGroup(SG[j]);
        }
        int count = -1;
        for (int i = 0; i < numofchild; i++)
        {
            if (i % 2 == 0)
                count++;
            SG[count].addComponent(Overall_Child[i]);
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
        for (int i = 0; i < numofchild; i++)
        {
            if (i % 2 == 0)
                count++;
            PGV[count].addComponent(Overall_Child[i]);
        }
        jPanel4Layout.setVerticalGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING).addGroup(SGV));
        //System.out.println("Child Num " + numofchild);
    }

    public void ProcessHandler(){
        int inbyte2;
        int diskid;
        byte[] usage = new byte[100];
        int freesize;
        int freeoffset;
        try{
            inbyte2 = Global.sd.ReadByte();
            diskid = Global.sd.ReadInt();
        } catch (IOException e) {
            return;
        }
        if (inbyte2 == 0)
        {
            try{
                freesize = Global.sd.ReadInt();
                freeoffset = Global.sd.ReadInt();
                Global.sd.ReadFully(usage, 0, 100);
                //System.out.println("Disk " + diskid);
            } catch (IOException e) {
                return;
            }
            SwingUtilities.invokeLater(new ChangeLabel(diskid,freesize,freeoffset,usage));
        }
        else if(inbyte2 == 1){
            boolean normal = true;
            try{
                inbyte2 = Global.sd.ReadByte();
            } catch (IOException e) {
                return;
            }
            if(inbyte2 == 1) normal = false;
            else if(inbyte2 == 0) normal = true;
            SwingUtilities.invokeLater(new Fail(diskid,normal));
        }
    }

    /**
     * @brief   Update the GUI after the event-dispatching thread
     */
    private class ChangeLabel implements Runnable{
        private int _diskid;
        private byte [] _usage;
        private int _freesize;
        private int _freeoffset;
        ChangeLabel(int diskid, int freesize, int freeoffset, byte[] usage){
            _diskid = diskid;
            _usage = usage;
            _freesize = freesize;
            _freeoffset = freeoffset;
        }
        public void run() {
            Child[_diskid].setLabel(0,_usage);
            Child[_diskid].setFreeSize(_freesize);
            Child[_diskid].setFreeOffset(_freeoffset);
            Overall_Child[_diskid].setLabel(0, _usage);
        }
    }

    private class Fail implements Runnable{
        private int _diskid;
        private boolean _normal;
        Fail(int diskid, boolean normal){
            _diskid = diskid;
            _normal = normal;
        }
        public void run() {
            Child[_diskid].setFail(_normal);
            Overall_Child[_diskid].setFail(_normal);
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

        Disk_Details_Tab = new javax.swing.JTabbedPane();
        jPanel1 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        Blue_backup = new java.awt.Label();
        Blue_text_backup = new javax.swing.JLabel();
        Pink_backup = new java.awt.Label();
        Pink_text_backup = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        Overall_Panel = new javax.swing.JPanel();

        Disk_Details_Tab.setTabLayoutPolicy(javax.swing.JTabbedPane.SCROLL_TAB_LAYOUT);
        Disk_Details_Tab.setTabPlacement(javax.swing.JTabbedPane.LEFT);
        Disk_Details_Tab.setMaximumSize(new java.awt.Dimension(640, 360));
        Disk_Details_Tab.setMinimumSize(new java.awt.Dimension(640, 360));
        Disk_Details_Tab.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                Disk_Details_TabStateChanged(evt);
            }
        });

        jPanel1.setMaximumSize(new java.awt.Dimension(640, 400));
        jPanel1.setMinimumSize(new java.awt.Dimension(640, 400));

        jScrollPane1.setMaximumSize(new java.awt.Dimension(500, 360));
        jScrollPane1.setMinimumSize(new java.awt.Dimension(500, 360));
        jScrollPane1.setPreferredSize(new java.awt.Dimension(500, 360));

        jPanel2.setBackground(new java.awt.Color(220, 234, 247));
        jPanel2.setMaximumSize(new java.awt.Dimension(500, 300));
        jPanel2.setMinimumSize(new java.awt.Dimension(500, 300));
        jPanel2.setPreferredSize(new java.awt.Dimension(500, 400));

        Blue_backup.setBackground(new java.awt.Color(0, 102, 153));
        Blue_backup.setCursor(new java.awt.Cursor(java.awt.Cursor.DEFAULT_CURSOR));

        Blue_text_backup.setText("Used Space");

        Pink_backup.setBackground(new java.awt.Color(255, 0, 255));
        Pink_backup.setCursor(new java.awt.Cursor(java.awt.Cursor.DEFAULT_CURSOR));

        Pink_text_backup.setText("Free Space");

        jLabel1.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Resourses/Disk_Details.png"))); // NOI18N

        Overall_Panel.setPreferredSize(new java.awt.Dimension(500, 300));
        Overall_Panel.setRequestFocusEnabled(false);

        javax.swing.GroupLayout Overall_PanelLayout = new javax.swing.GroupLayout(Overall_Panel);
        Overall_Panel.setLayout(Overall_PanelLayout);
        Overall_PanelLayout.setHorizontalGroup(
            Overall_PanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 551, Short.MAX_VALUE)
        );
        Overall_PanelLayout.setVerticalGroup(
            Overall_PanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 317, Short.MAX_VALUE)
        );

        jScrollPane2.setViewportView(Overall_Panel);

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 553, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addComponent(Blue_backup, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(Blue_text_backup))
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addComponent(Pink_backup, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(Pink_text_backup)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 84, Short.MAX_VALUE)
                        .addComponent(jLabel1)))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(Blue_backup, 0, 0, Short.MAX_VALUE)
                            .addComponent(Blue_text_backup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(Pink_backup, javax.swing.GroupLayout.Alignment.TRAILING, 0, 0, Short.MAX_VALUE)
                            .addComponent(Pink_text_backup, javax.swing.GroupLayout.Alignment.TRAILING))
                        .addGap(39, 39, 39))
                    .addComponent(jLabel1, javax.swing.GroupLayout.Alignment.TRAILING))
                .addGap(43, 43, 43))
        );

        jScrollPane1.setViewportView(jPanel2);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 575, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 465, Short.MAX_VALUE)
                .addContainerGap())
        );

        Disk_Details_Tab.addTab("Overall", jPanel1);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(Disk_Details_Tab, javax.swing.GroupLayout.DEFAULT_SIZE, 640, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(Disk_Details_Tab, javax.swing.GroupLayout.DEFAULT_SIZE, 480, Short.MAX_VALUE)
        );
    }// </editor-fold>//GEN-END:initComponents

    private void Disk_Details_TabStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_Disk_Details_TabStateChanged
        // TODO add your handling code here:
        /*
            Global.lock.lock();
        try {
            Global.sd.SendByte((byte)1);
            Global.sd.SendInt(Disk_Details_Tab.getSelectedIndex());
        } finally {
            Global.lock.unlock();
        }
         */
    }//GEN-LAST:event_Disk_Details_TabStateChanged

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private java.awt.Label Blue_backup;
    private javax.swing.JLabel Blue_text_backup;
    private javax.swing.JTabbedPane Disk_Details_Tab;
    private javax.swing.JPanel Overall_Panel;
    private java.awt.Label Pink_backup;
    private javax.swing.JLabel Pink_text_backup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    // End of variables declaration//GEN-END:variables
}
