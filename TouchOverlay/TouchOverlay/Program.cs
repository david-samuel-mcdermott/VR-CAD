using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TouchOverlay
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            //SynchronousSocketClient.StartClient();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new OverlayForm());
            //TODO: launch client socket to pass information to Rhino
            //TODO: expose Windows message pump to get touch gestures
        }
    }
}
