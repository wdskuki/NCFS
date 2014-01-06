/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package FYP;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 *
 * @author user
 */
public class Network_Layer {
    private Socket sd = null;
    private DataInputStream Inputsd;
    private DataOutputStream Outputsd;
        
    public Network_Layer(String ServerAddress, int port){
        InputStream in;
        OutputStream out;
        try{
            sd = new Socket(ServerAddress, port);
            in = sd.getInputStream();
            out = sd.getOutputStream();
            Inputsd = new DataInputStream(in);
            Outputsd = new DataOutputStream(out);
        } catch (UnknownHostException e) {
            System.err.println("Don't know about " + ServerAddress);
            //System.exit(1);
        } catch (IOException e) {
            System.err.println("Couldn't connect to " + ServerAddress);
            //System.exit(1);
        }
    }

    public byte ReadByte() throws IOException{
        return Inputsd.readByte();
    }

    public int ReadInt() throws IOException{
        return Inputsd.readInt();
    }

    public long ReadLong() throws IOException{
        return Inputsd.readLong();
    }

    public void ReadFully(byte[] b, int off, int len) throws IOException{
         Inputsd.readFully(b, off, len);
         return;
    }
    
    public String ReadString(){
         byte bytearr[] = new byte[1024];
         String str;
         byte inbyte;
         int count = 0;
         while(true){
             try{
                 inbyte = Global.sd.ReadByte();
                 if(inbyte == '\\'){
                     str = new String(bytearr,0,count);
                     break;
                 }
                 bytearr[count] = inbyte;
                 count++;
             } catch (IOException e) {
                 str = null;
             }
         }
//         System.out.println(str);
         return str;
    }

    public String[] ReadString3() throws IOException{
        String[] str = {"Disk 1", "Disk 2", "Disk 3", "Disk 4"};
        return str;
    }

    public String ReadString2() throws IOException{
        String str = "/C/ABC/dir1/";
        return str;
    }

    public void SendString(String str){
        try{
        //str = str + "\\";
        byte[] bytearr = str.getBytes();
        SendInt(bytearr.length);
        //System.out.print("Send String:");
        //for(int i = 0; i < bytearr.length; ++i)
        //    System.out.print(bytearr[i]);
        //System.out.println();
        SendFully(bytearr,0,bytearr.length);
        //SendByte(46);
        //Outputsd.writeBytes(str);
        //Outputsd.writeBytes("\\");
        } catch (IOException e) {
            return;
        }
        return;
    }

    public void SendLong(long length){
        try{
            Outputsd.writeLong(length);
        } catch (IOException e) {
            return;
        }
    }

    public void SendByte(byte no){
        try{
            Outputsd.writeByte(no);
        } catch (IOException e) {
            return;
        }
    }

    public void SendFully(byte[] b, int off, int len) throws IOException{
         Outputsd.write(b, off, len);
         return;
    }

    public void SendInt(int no){
        try{
            Outputsd.writeInt(no);
        } catch (IOException e) {
            return;
        }
    }
}