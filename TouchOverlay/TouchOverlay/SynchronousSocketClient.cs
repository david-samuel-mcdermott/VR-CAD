using System;  
using System.Net;  
using System.Net.Sockets;  
using System.Text;  
  
public class SynchronousSocketClient {

    private static Socket sender;

    public static void StartClient() {  
   
        // Connect to a remote device.  
        try {  
            // Establish the remote endpoint for the socket.  
            // Change field below to connect to rhino.  
            IPAddress ipAddress = IPAddress.Parse("127.0.0.1");  
            IPEndPoint remoteEP = new IPEndPoint(ipAddress, 11000);  
  
            // Create a TCP/IP  socket.  
            sender = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp );  
  
            // Connect the socket to the remote endpoint. Catch any errors.  
            try {  
                sender.Connect(remoteEP);  
            } catch (ArgumentNullException ane) {  
                Console.WriteLine("ArgumentNullException : {0}",ane.ToString());  
            } catch (SocketException se) {  
                Console.WriteLine("SocketException : {0}",se.ToString());  
            } catch (Exception e) {  
                Console.WriteLine("Unexpected exception : {0}", e.ToString());  
            }  
  
        } catch (Exception e) {  
            Console.WriteLine( e.ToString());  
        }  
    }  
  
    public static void Close()
    {
        sender.Shutdown(SocketShutdown.Both);
        sender.Close();
    }

    public static int SendPOST(string payload)
    {
        string header = "POST /UI HTTP/1.1\n\rHost: 127.0.0.1\n\rContent-Type: text\n\rContent-Length: " + payload.Length + "\n\r\n\r" + payload + "\n\r\n\r";
        return sender.Send(Encoding.ASCII.GetBytes(header));
    } 

    public static bool RecvResponse()
    {
        byte[] buffer = new byte[sender.ReceiveBufferSize];
        sender.Receive(buffer);
        string response = Encoding.ASCII.GetString(buffer);
        if (response.StartsWith("200 OK"))
        {
            return true;
        }
        return false;
    }

}  
