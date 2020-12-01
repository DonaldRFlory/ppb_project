using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TestApp
{
    public partial class EntryDialog : Form
    {
        public EntryDialog(string Prompt)
        {																				
            InitializeComponent();
			LPrompt.Text = Prompt;
    		BAccept.DialogResult = DialogResult.OK;
		   	BCancel.DialogResult = DialogResult.Cancel;
        }

		public string Input;
		private void TBEntry_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                Input = TBEntry.Text;
				DialogResult = DialogResult.OK;
			}

        }

        private void BAccept_Click(object sender, EventArgs e)
        {
            Input = TBEntry.Text;
        }

        private void BCancel_Click(object sender, EventArgs e)
        {

        }
    }
}

