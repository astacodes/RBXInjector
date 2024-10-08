using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace RBXInjector
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private DispatcherTimer processTimer;
        private List<ProcessItem> currentProcessItems = new List<ProcessItem>();
        private string selectedDllPath;
        private int selectedPID;

        public MainWindow()
        {
            InitializeComponent();

            processTimer = new DispatcherTimer();
            processTimer.Interval = TimeSpan.FromSeconds(1);
            processTimer.Tick += ProcessTimer_Tick;
            processTimer.Start();
        }

        private class ProcessItem
        {
            public string ProcessName { get; set; }
            public int ProcessID { get; set; }

            public ProcessItem(string name, int id)
            {
                ProcessName = name;
                ProcessID = id;
            }
            public override string ToString()
            {
                return $"{ProcessName}.exe (PID: {ProcessID})";
            }
        }

        private void ProcessTimer_Tick(object sender, EventArgs e)
        {
            Process[] robloxProcesses = Process.GetProcessesByName("RobloxPlayerBeta");
            HashSet<int> existingProcessIds = new HashSet<int>(currentProcessItems.Select(item => item.ProcessID));

            foreach (Process proc in robloxProcesses)
            {
                if (!existingProcessIds.Contains(proc.Id))
                {
                    ProcessItem newItem = new ProcessItem(proc.ProcessName, proc.Id);
                    comboBox1.Items.Add(newItem);
                    currentProcessItems.Add(newItem);
                }
            }

            for (int i = currentProcessItems.Count - 1; i >= 0; i--)
            {
                if (!robloxProcesses.Any(p => p.Id == currentProcessItems[i].ProcessID))
                {
                    comboBox1.Items.Remove(currentProcessItems[i]);
                    currentProcessItems.RemoveAt(i);
                }
            }
        }

        private void ComboBox_Loaded(object sender, RoutedEventArgs e)
        {
            ComboBox comboBox = (ComboBox)sender;
            ToggleButton toggleButton = comboBox.Template.FindName("toggleButton", comboBox) as ToggleButton;
            if (toggleButton != null)
            {
                Border border = toggleButton.Template.FindName("templateRoot", toggleButton) as Border;
                if (border != null)
                    border.Background = (Brush)new BrushConverter().ConvertFromString("#1e1e1e");
            }
        }

        private void comboBox1_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (comboBox1.SelectedItem is ProcessItem selectedProcess)
            {
                selectedPID = selectedProcess.ProcessID;
                MessageBox.Show($"Selected Process PID: {selectedPID}");
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog openFileDialog = new Microsoft.Win32.OpenFileDialog();
            openFileDialog.Filter = "DLL Files (*.dll)|*.dll";
            if (openFileDialog.ShowDialog() == true)
            {
                selectedDllPath = openFileDialog.FileName;
                Button button = sender as Button;
                if (button != null)
                {
                    textBox1.Text = System.IO.Path.GetFileName(selectedDllPath);
                }
            }
        }

        private void comboBox2_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (comboBox2.SelectedItem is ComboBoxItem selectedItem)
            {
                string itemName = selectedItem.Content.ToString();

                if (itemName == "WinTrust Patch (SetWindowsHookEx)")
                {
                    winTrustCallback.Visibility = Visibility.Visible;
                    winTrustCallbackText.Visibility = Visibility.Visible;
                } else
                {
                    winTrustCallback.Visibility = Visibility.Hidden;
                    winTrustCallbackText.Visibility = Visibility.Hidden;
                }
            }
        }

        private void InjectDLL_Click(object sender, RoutedEventArgs e)
        {
            if (comboBox2.SelectedItem is ComboBoxItem selectedItem)
            {
                string itemName = selectedItem.Content.ToString();

                if (itemName == "WinTrust Patch (SetWindowsHookEx)")
                {
                    string exePath = System.IO.Path.Combine("bin", "WinTrust Patch Injector.exe");

                    string arg1 = selectedDllPath;
                    string arg2 = winTrustCallback.Text;
                    string arg3 = selectedPID.ToString();
                    string arguments = $"{arg1} {arg2} {arg3}";

                    ProcessStartInfo startInfo = new ProcessStartInfo
                    {
                        FileName = exePath,
                        Arguments = arguments,
                        UseShellExecute = true
                    };

                    try
                    {
                        Process.Start(startInfo);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show($"Error starting process: {ex.Message}");
                    }

                    return;
                }
            }
        }
    }
}
