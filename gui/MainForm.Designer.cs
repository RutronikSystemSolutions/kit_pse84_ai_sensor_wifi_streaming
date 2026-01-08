namespace kit_pse84_ai_wifi_streaming
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            openConnectionButton = new Button();
            timeSignalView = new kit_pse84_ai_wifi_streaming.Views.TimeSignalView();
            cameraPictureBox = new PictureBox();
            connStateTextBox = new TextBox();
            logTextBox = new TextBox();
            logsLabel = new Label();
            mainSplitContainer = new SplitContainer();
            radarSplitContainer = new SplitContainer();
            amplitudeTimeView = new kit_pse84_ai_wifi_streaming.Views.AmplitudeTimeView();
            dataLoggerTabControl = new TabControl();
            dataLoggerTabPage = new TabPage();
            storedTextBox = new TextBox();
            storedLabel = new Label();
            stopLoggerButton = new Button();
            startLoggerButton = new Button();
            selectPathButton = new Button();
            filePathTextBox = new TextBox();
            storeToFileLabel = new Label();
            ((System.ComponentModel.ISupportInitialize)cameraPictureBox).BeginInit();
            ((System.ComponentModel.ISupportInitialize)mainSplitContainer).BeginInit();
            mainSplitContainer.Panel1.SuspendLayout();
            mainSplitContainer.Panel2.SuspendLayout();
            mainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)radarSplitContainer).BeginInit();
            radarSplitContainer.Panel1.SuspendLayout();
            radarSplitContainer.Panel2.SuspendLayout();
            radarSplitContainer.SuspendLayout();
            dataLoggerTabControl.SuspendLayout();
            dataLoggerTabPage.SuspendLayout();
            SuspendLayout();
            // 
            // openConnectionButton
            // 
            openConnectionButton.Location = new Point(10, 9);
            openConnectionButton.Margin = new Padding(3, 2, 3, 2);
            openConnectionButton.Name = "openConnectionButton";
            openConnectionButton.Size = new Size(152, 22);
            openConnectionButton.TabIndex = 0;
            openConnectionButton.Text = "Open connection";
            openConnectionButton.UseVisualStyleBackColor = true;
            openConnectionButton.Click += openConnectionButton_Click;
            // 
            // timeSignalView
            // 
            timeSignalView.BorderStyle = BorderStyle.FixedSingle;
            timeSignalView.Dock = DockStyle.Fill;
            timeSignalView.Location = new Point(0, 0);
            timeSignalView.Margin = new Padding(3, 2, 3, 2);
            timeSignalView.Name = "timeSignalView";
            timeSignalView.Size = new Size(696, 205);
            timeSignalView.TabIndex = 1;
            // 
            // cameraPictureBox
            // 
            cameraPictureBox.BorderStyle = BorderStyle.FixedSingle;
            cameraPictureBox.Dock = DockStyle.Fill;
            cameraPictureBox.Location = new Point(0, 0);
            cameraPictureBox.Margin = new Padding(3, 2, 3, 2);
            cameraPictureBox.Name = "cameraPictureBox";
            cameraPictureBox.Size = new Size(350, 415);
            cameraPictureBox.SizeMode = PictureBoxSizeMode.Zoom;
            cameraPictureBox.TabIndex = 2;
            cameraPictureBox.TabStop = false;
            // 
            // connStateTextBox
            // 
            connStateTextBox.Location = new Point(168, 10);
            connStateTextBox.Name = "connStateTextBox";
            connStateTextBox.ReadOnly = true;
            connStateTextBox.Size = new Size(100, 23);
            connStateTextBox.TabIndex = 3;
            // 
            // logTextBox
            // 
            logTextBox.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            logTextBox.Location = new Point(619, 475);
            logTextBox.Multiline = true;
            logTextBox.Name = "logTextBox";
            logTextBox.ReadOnly = true;
            logTextBox.ScrollBars = ScrollBars.Vertical;
            logTextBox.Size = new Size(443, 144);
            logTextBox.TabIndex = 4;
            // 
            // logsLabel
            // 
            logsLabel.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            logsLabel.AutoSize = true;
            logsLabel.Location = new Point(619, 457);
            logsLabel.Name = "logsLabel";
            logsLabel.Size = new Size(35, 15);
            logsLabel.TabIndex = 5;
            logsLabel.Text = "Logs:";
            // 
            // mainSplitContainer
            // 
            mainSplitContainer.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            mainSplitContainer.Location = new Point(12, 39);
            mainSplitContainer.Name = "mainSplitContainer";
            // 
            // mainSplitContainer.Panel1
            // 
            mainSplitContainer.Panel1.Controls.Add(cameraPictureBox);
            // 
            // mainSplitContainer.Panel2
            // 
            mainSplitContainer.Panel2.Controls.Add(radarSplitContainer);
            mainSplitContainer.Size = new Size(1050, 415);
            mainSplitContainer.SplitterDistance = 350;
            mainSplitContainer.TabIndex = 6;
            // 
            // radarSplitContainer
            // 
            radarSplitContainer.Dock = DockStyle.Fill;
            radarSplitContainer.Location = new Point(0, 0);
            radarSplitContainer.Name = "radarSplitContainer";
            radarSplitContainer.Orientation = Orientation.Horizontal;
            // 
            // radarSplitContainer.Panel1
            // 
            radarSplitContainer.Panel1.Controls.Add(timeSignalView);
            // 
            // radarSplitContainer.Panel2
            // 
            radarSplitContainer.Panel2.Controls.Add(amplitudeTimeView);
            radarSplitContainer.Size = new Size(696, 415);
            radarSplitContainer.SplitterDistance = 205;
            radarSplitContainer.TabIndex = 4;
            // 
            // amplitudeTimeView
            // 
            amplitudeTimeView.BorderStyle = BorderStyle.FixedSingle;
            amplitudeTimeView.Dock = DockStyle.Fill;
            amplitudeTimeView.Location = new Point(0, 0);
            amplitudeTimeView.Name = "amplitudeTimeView";
            amplitudeTimeView.Size = new Size(696, 206);
            amplitudeTimeView.TabIndex = 0;
            // 
            // dataLoggerTabControl
            // 
            dataLoggerTabControl.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            dataLoggerTabControl.Controls.Add(dataLoggerTabPage);
            dataLoggerTabControl.Location = new Point(10, 460);
            dataLoggerTabControl.Name = "dataLoggerTabControl";
            dataLoggerTabControl.SelectedIndex = 0;
            dataLoggerTabControl.Size = new Size(603, 159);
            dataLoggerTabControl.TabIndex = 7;
            // 
            // dataLoggerTabPage
            // 
            dataLoggerTabPage.Controls.Add(storedTextBox);
            dataLoggerTabPage.Controls.Add(storedLabel);
            dataLoggerTabPage.Controls.Add(stopLoggerButton);
            dataLoggerTabPage.Controls.Add(startLoggerButton);
            dataLoggerTabPage.Controls.Add(selectPathButton);
            dataLoggerTabPage.Controls.Add(filePathTextBox);
            dataLoggerTabPage.Controls.Add(storeToFileLabel);
            dataLoggerTabPage.Location = new Point(4, 24);
            dataLoggerTabPage.Name = "dataLoggerTabPage";
            dataLoggerTabPage.Padding = new Padding(3);
            dataLoggerTabPage.Size = new Size(595, 131);
            dataLoggerTabPage.TabIndex = 0;
            dataLoggerTabPage.Text = "Data Logger";
            dataLoggerTabPage.UseVisualStyleBackColor = true;
            // 
            // storedTextBox
            // 
            storedTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            storedTextBox.Location = new Point(489, 49);
            storedTextBox.Name = "storedTextBox";
            storedTextBox.ReadOnly = true;
            storedTextBox.Size = new Size(100, 23);
            storedTextBox.TabIndex = 6;
            // 
            // storedLabel
            // 
            storedLabel.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            storedLabel.AutoSize = true;
            storedLabel.Location = new Point(407, 52);
            storedLabel.Name = "storedLabel";
            storedLabel.Size = new Size(76, 15);
            storedLabel.TabIndex = 5;
            storedLabel.Text = "Stored so far:";
            // 
            // stopLoggerButton
            // 
            stopLoggerButton.Enabled = false;
            stopLoggerButton.Location = new Point(87, 48);
            stopLoggerButton.Name = "stopLoggerButton";
            stopLoggerButton.Size = new Size(75, 23);
            stopLoggerButton.TabIndex = 4;
            stopLoggerButton.Text = "Stop";
            stopLoggerButton.UseVisualStyleBackColor = true;
            stopLoggerButton.Click += stopLoggerButton_Click;
            // 
            // startLoggerButton
            // 
            startLoggerButton.Enabled = false;
            startLoggerButton.Location = new Point(6, 48);
            startLoggerButton.Name = "startLoggerButton";
            startLoggerButton.Size = new Size(75, 23);
            startLoggerButton.TabIndex = 3;
            startLoggerButton.Text = "Start";
            startLoggerButton.UseVisualStyleBackColor = true;
            startLoggerButton.Click += startLoggerButton_Click;
            // 
            // selectPathButton
            // 
            selectPathButton.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            selectPathButton.Location = new Point(554, 5);
            selectPathButton.Name = "selectPathButton";
            selectPathButton.Size = new Size(35, 23);
            selectPathButton.TabIndex = 2;
            selectPathButton.Text = "...";
            selectPathButton.UseVisualStyleBackColor = true;
            selectPathButton.Click += selectPathButton_Click;
            // 
            // filePathTextBox
            // 
            filePathTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            filePathTextBox.Location = new Point(82, 6);
            filePathTextBox.Name = "filePathTextBox";
            filePathTextBox.ReadOnly = true;
            filePathTextBox.Size = new Size(466, 23);
            filePathTextBox.TabIndex = 1;
            // 
            // storeToFileLabel
            // 
            storeToFileLabel.AutoSize = true;
            storeToFileLabel.Location = new Point(6, 9);
            storeToFileLabel.Name = "storeToFileLabel";
            storeToFileLabel.Size = new Size(70, 15);
            storeToFileLabel.TabIndex = 0;
            storeToFileLabel.Text = "Store to file:";
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1074, 631);
            Controls.Add(dataLoggerTabControl);
            Controls.Add(mainSplitContainer);
            Controls.Add(logsLabel);
            Controls.Add(logTextBox);
            Controls.Add(connStateTextBox);
            Controls.Add(openConnectionButton);
            Icon = (Icon)resources.GetObject("$this.Icon");
            Margin = new Padding(3, 2, 3, 2);
            Name = "MainForm";
            Text = "Rutronik - KIT_PSE84_AI_WIFI_STREAMING";
            ((System.ComponentModel.ISupportInitialize)cameraPictureBox).EndInit();
            mainSplitContainer.Panel1.ResumeLayout(false);
            mainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)mainSplitContainer).EndInit();
            mainSplitContainer.ResumeLayout(false);
            radarSplitContainer.Panel1.ResumeLayout(false);
            radarSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)radarSplitContainer).EndInit();
            radarSplitContainer.ResumeLayout(false);
            dataLoggerTabControl.ResumeLayout(false);
            dataLoggerTabPage.ResumeLayout(false);
            dataLoggerTabPage.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button openConnectionButton;
        private Views.TimeSignalView timeSignalView;
        private PictureBox cameraPictureBox;
        private TextBox connStateTextBox;
        private TextBox logTextBox;
        private Label logsLabel;
        private SplitContainer mainSplitContainer;
        private SplitContainer radarSplitContainer;
        private Views.AmplitudeTimeView amplitudeTimeView;
        private TabControl dataLoggerTabControl;
        private TabPage dataLoggerTabPage;
        private TextBox filePathTextBox;
        private Label storeToFileLabel;
        private Button selectPathButton;
        private Button startLoggerButton;
        private Button stopLoggerButton;
        private TextBox storedTextBox;
        private Label storedLabel;
    }
}
