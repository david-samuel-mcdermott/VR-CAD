namespace TouchOverlay
{
    partial class OverlayForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        //Note: Any code in this region will be overwritten if the visual editor is used
        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.rightFlowLayout = new System.Windows.Forms.FlowLayoutPanel();
            this.exitButton = new System.Windows.Forms.Button();
            this.leftFlowLayout = new System.Windows.Forms.FlowLayoutPanel();
            this.exampleButton = new System.Windows.Forms.Button();
            this.rightFlowLayout.SuspendLayout();
            this.leftFlowLayout.SuspendLayout();
            this.SuspendLayout();
            // 
            // rightFlowLayout
            // 
            this.rightFlowLayout.Controls.Add(this.exitButton);
            this.rightFlowLayout.Dock = System.Windows.Forms.DockStyle.Right;
            this.rightFlowLayout.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.rightFlowLayout.Location = new System.Drawing.Point(745, 0);
            this.rightFlowLayout.Name = "rightFlowLayout";
            this.rightFlowLayout.Size = new System.Drawing.Size(55, 450);
            this.rightFlowLayout.TabIndex = 0;
            // 
            // exitButton
            // 
            this.exitButton.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.exitButton.Location = new System.Drawing.Point(3, 3);
            this.exitButton.Name = "exitButton";
            this.exitButton.Size = new System.Drawing.Size(50, 50);
            this.exitButton.TabIndex = 0;
            this.exitButton.Text = "Exit";
            this.exitButton.UseVisualStyleBackColor = true;
            // 
            // leftFlowLayout
            // 
            this.leftFlowLayout.Controls.Add(this.exampleButton);
            this.leftFlowLayout.Dock = System.Windows.Forms.DockStyle.Left;
            this.leftFlowLayout.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.leftFlowLayout.Location = new System.Drawing.Point(0, 0);
            this.leftFlowLayout.Name = "leftFlowLayout";
            this.leftFlowLayout.Size = new System.Drawing.Size(55, 450);
            this.leftFlowLayout.TabIndex = 1;
            // 
            // exampleButton
            // 
            this.exampleButton.Location = new System.Drawing.Point(3, 3);
            this.exampleButton.Name = "exampleButton";
            this.exampleButton.Size = new System.Drawing.Size(50, 50);
            this.exampleButton.TabIndex = 0;
            this.exampleButton.Text = "Test Button";
            this.exampleButton.UseVisualStyleBackColor = true;
            // 
            // OverlayForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.LightGreen;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.ControlBox = false;
            this.Controls.Add(this.leftFlowLayout);
            this.Controls.Add(this.rightFlowLayout);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "OverlayForm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Touch Overlay";
            this.TopMost = true;
            this.TransparencyKey = System.Drawing.Color.LightGreen;
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.OverlayForm_Load);
            this.rightFlowLayout.ResumeLayout(false);
            this.leftFlowLayout.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.FlowLayoutPanel rightFlowLayout;
        private System.Windows.Forms.FlowLayoutPanel leftFlowLayout;
        private System.Windows.Forms.Button exitButton;
        private System.Windows.Forms.Button exampleButton;
    }
}

