using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TouchOverlay
{
    public partial class OverlayForm : Form
    {

        public OverlayForm()
        {
            InitializeComponent();

            //TODO: Add any custom code for UI components here
            this.exampleButton.Click += ExampleButton_Click;
            this.exitButton.Click += ExitButton_Click;

        }


        private void ExampleButton_Click(object sender, System.EventArgs e)
        {
            //This is an empty body for a click event; throw exception can be replaced with functionality
            throw new System.NotImplementedException();
        }

        private void ExitButton_Click(object sender, System.EventArgs e) => System.Windows.Forms.Application.Exit();

        private void OverlayForm_Load(object sender, EventArgs e)
        {

        }
    }
}
