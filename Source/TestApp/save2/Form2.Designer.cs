namespace TestApp
{
    partial class EntryDialog
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.TBEntry = new System.Windows.Forms.TextBox();
            this.BAccept = new System.Windows.Forms.Button();
            this.BCancel = new System.Windows.Forms.Button();
            this.LPrompt = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // TBEntry
            // 
            this.TBEntry.AcceptsReturn = true;
            this.TBEntry.Location = new System.Drawing.Point(102, 43);
            this.TBEntry.Multiline = true;
            this.TBEntry.Name = "TBEntry";
            this.TBEntry.Size = new System.Drawing.Size(144, 22);
            this.TBEntry.TabIndex = 1;
            this.TBEntry.KeyDown += new System.Windows.Forms.KeyEventHandler(this.TBEntry_KeyDown);
            // 
            // BAccept
            // 
            this.BAccept.Location = new System.Drawing.Point(102, 96);
            this.BAccept.Name = "BAccept";
            this.BAccept.Size = new System.Drawing.Size(53, 34);
            this.BAccept.TabIndex = 2;
            this.BAccept.Text = "Accept";
            this.BAccept.UseVisualStyleBackColor = true;
            this.BAccept.Click += new System.EventHandler(this.BAccept_Click);
            // 
            // BCancel
            // 
            this.BCancel.Location = new System.Drawing.Point(187, 96);
            this.BCancel.Name = "BCancel";
            this.BCancel.Size = new System.Drawing.Size(53, 34);
            this.BCancel.TabIndex = 3;
            this.BCancel.Text = "Cancel";
            this.BCancel.UseVisualStyleBackColor = true;
            this.BCancel.Click += new System.EventHandler(this.BCancel_Click);
            // 
            // LPrompt
            // 
            this.LPrompt.AutoSize = true;
            this.LPrompt.Location = new System.Drawing.Point(99, 27);
            this.LPrompt.Name = "LPrompt";
            this.LPrompt.Size = new System.Drawing.Size(35, 13);
            this.LPrompt.TabIndex = 0;
            this.LPrompt.Text = "label1";
            this.LPrompt.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // EntryDialog
            // 
            this.AcceptButton = this.BAccept;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(346, 176);
            this.Controls.Add(this.BCancel);
            this.Controls.Add(this.BAccept);
            this.Controls.Add(this.TBEntry);
            this.Controls.Add(this.LPrompt);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "EntryDialog";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox TBEntry;
        private System.Windows.Forms.Button BAccept;
        private System.Windows.Forms.Button BCancel;
        private System.Windows.Forms.Label LPrompt;
    }
}