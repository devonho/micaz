using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace uispgui
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            Assembly assem = Assembly.GetEntryAssembly();
            AssemblyName assemName = assem.GetName();
            Version ver = assemName.Version;
            this.Text = String.Format(assemName.Name + " " + ver.Major + "." + ver.Minor + "." + ver.Revision + " Build " + ver.Build);

            EnumerateComPorts();
        }

        public void EnumerateComPorts()
        {
            String[] portnames = SerialPort.GetPortNames();

            foreach(String name in portnames)
            {
                comboBox1.Items.Add(name);
            }
            comboBox1.SelectedIndex = 1;
        }

        public void InvokeUISP()
        {
            StringBuilder outputBuilder = new StringBuilder();
            richTextBox1.Text = String.Empty;

            ProcessStartInfo processStartInfo = new ProcessStartInfo();
            processStartInfo.CreateNoWindow = true; 
            processStartInfo.RedirectStandardOutput = false;
            processStartInfo.RedirectStandardInput = false;
            processStartInfo.UseShellExecute = true;
            processStartInfo.Arguments = @"-dprog=mib510 -v -dserial=" + selectedPortName + @" -dpart=ATmega128  --erase --upload if=blink.srec --verify";
            //processStartInfo.Arguments = @"--help";
            processStartInfo.FileName = "uisp.exe";

            Process process = new Process();
            process.StartInfo = processStartInfo;
            // enable raising events because Process does not raise events by default
            process.EnableRaisingEvents = true;
            // attach the event handler for OutputDataReceived before starting the process
            process.OutputDataReceived += new DataReceivedEventHandler
            (
                delegate(object sender, DataReceivedEventArgs e)
                {
                    // append the new data to the data already read-in
                    outputBuilder.Append(e.Data + "\n");
                }
            );
            // start the process
            // then begin asynchronously reading the output
            // then wait for the process to exit
            // then cancel asynchronously reading the output
            process.Start();
            //process.BeginOutputReadLine();
            process.WaitForExit();
            //process.CancelOutputRead();

            // use the output
            richTextBox1.Text = outputBuilder.ToString();
        }


        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cb = (ComboBox)sender;
            String selectedItem = (String)comboBox1.Items[cb.SelectedIndex];

            selectedItem = selectedItem.Replace("COM", String.Empty);

            int portNum = Convert.ToInt16(selectedItem);
            portNum = portNum - 1;

            selectedPortName = @"/dev/ttyS" + portNum;
        }

        String selectedPortName;

        private void button1_Click(object sender, EventArgs e)
        {
            if (selectedPortName != null)
            {
                InvokeUISP();
            }

        }
    }


}
