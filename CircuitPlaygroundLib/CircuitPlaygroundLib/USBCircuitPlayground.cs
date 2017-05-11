using HidSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;


namespace CircuitPlaygroundLib
{
    public class USBCircuitPlayground : ICircuitPlayground, ILab3
    {
        private List<stats> Stats = new List<stats>();
        private List<AccelData> past = new List<AccelData>();
        private HidDeviceLoader deviceLoader;
        private IEnumerable<HidDevice> devices;
        private HidDevice hidDevice;

        private HidStream stream;
        private AccelData accelData;
        private AccelData oldAccelData;
        private float tempC;
        private float tempF;
        private ushort light;
        private ushort sound;
        private bool leftButton;
        private bool rightButton;
        private bool slideSwitch;
        private bool save = true;

        public bool IsOpen
        {
            get
            {
                return stream.CanRead;
            }
        }

        public AccelData ReadAccel
        {
            get
            {
                return accelData;
            }
        }

        public float ReadTempC
        {
            get
            {
                return tempC;
            }
        }

        public float ReadTempF
        {
            get
            {
                return tempF;
            }
        }

        public ushort ReadLight
        {
            get
            {
                return light;
            }
        }

        public ushort ReadSound
        {
            get
            {
                return sound;
            }
        }

        public bool ReadLeftButton
        {
            get
            {
                return leftButton;
            }
        }

        public bool ReadRightButton
        {
            get
            {
                return rightButton;
            }
        }

        public bool ReadSlideSwitch
        {
            get
            {
                return slideSwitch;
            }
        }

        public bool ReadHeld
        {
            get
            {
                return IsBeingHeld();
            }
        }

        public event EventHandler<EventArgs> DataReceived;

        /*public void SetPort(string portname)
        {
            port.PortName = portname;
        }*/

        public void ClearAllPixels()
        {
            if (stream.CanWrite)
            {
                byte[] buffer = new byte[64];
                buffer[0] = 0x00;
                buffer[1] = 0x02;
                stream.Write(buffer, 0, buffer.Length);
            }
        }

        public void Close()
        {
            if (stream.CanRead)
            {
                stream.Close();
            }
        }

        public string[] GetDevices()
        {
            deviceLoader = new HidDeviceLoader();
            devices = deviceLoader.GetDevices();
            string[] ports = new string[devices.Count()];
            var list = devices.ToList();
            for (int i=0;i<list.Count();i++)
            {
                ports[i] = list[i].ProductName;
            }
            //string[] devices ;
            if (ports.Length == 1)
            {
                Open(ports[0]);
            }
            return ports;


        }

        public bool Open(string device)
        {
            try
            {
                hidDevice = devices.Where(x => x.ProductName == device).FirstOrDefault();
                stream = hidDevice.Open();
                stream.ReadTimeout = System.Threading.Timeout.Infinite;
                Thread t = new Thread(read) { IsBackground = true };
                t.Start();
                return true;
            }
            catch
            {
                return false;
            }

        }

        private void read()
        {
            while (true)
            {
                if (stream.CanRead)
                {
                    var byteArray = stream.Read();
                    oldAccelData = accelData;
                    accelData.X = System.BitConverter.ToSingle(byteArray,1);
                    accelData.Y = System.BitConverter.ToSingle(byteArray, 5);
                    accelData.Z = System.BitConverter.ToSingle(byteArray, 9);
                    tempC = System.BitConverter.ToSingle(byteArray, 13);
                    tempF = System.BitConverter.ToSingle(byteArray, 17);
                    light = (ushort)System.BitConverter.ToSingle(byteArray, 21);
                    sound = (ushort)System.BitConverter.ToSingle(byteArray, 25);

                    if (byteArray[29] > 0)
                        leftButton = true;
                    else
                        leftButton = false;

                    if (byteArray[30] > 0)
                        rightButton = true;
                    else
                        rightButton = false;

                    if (byteArray[31] > 0)
                        slideSwitch = true;
                    else
                        slideSwitch = false;
                    if (save == true)
                    {
                        stats Stat;
                        Stat.accelData = accelData;
                        Stat.tempC = tempC;
                        Stat.tempF = tempF;
                        Stat.light = light;
                        Stat.sound = sound;
                        Stat.leftButton = leftButton;
                        Stat.rightButton = rightButton;
                        Stat.slideSwitch = slideSwitch;

                        Stats.Add(Stat);
                    }
                    DataReceived?.Invoke(this, new EventArgs());
                }
            }
        }

        /*private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            string data = port.ReadLine();

            try
            {

                float[] parts = data.Trim().Split(',').Select(float.Parse).ToArray();

                if (parts.Length == 10)
                {
                    oldAccelData = accelData;
                    accelData.X = parts[0];
                    accelData.Y = parts[1];
                    accelData.Z = parts[2];
                    tempC = parts[3];
                    tempF = parts[4];
                    light = (ushort)parts[5];
                    sound = (ushort)parts[6];

                    if (parts[7] > 0)
                        leftButton = true;
                    else
                        leftButton = false;

                    if (parts[8] > 0)
                        rightButton = true;
                    else
                        rightButton = false;

                    if (parts[9] > 0)
                        slideSwitch = true;
                    else
                        slideSwitch = false;
                    if (save == true)
                    {
                        stats Stat;
                        Stat.accelData = accelData;
                        Stat.tempC = tempC;
                        Stat.tempF = tempF;
                        Stat.light = light;
                        Stat.sound = sound;
                        Stat.leftButton = leftButton;
                        Stat.rightButton = rightButton;
                        Stat.slideSwitch = slideSwitch;

                        Stats.Add(Stat);
                    }
                    DataReceived?.Invoke(this, new EventArgs());

                }
            }
            catch { }
        }*/

        public void SetPixelColor(int pixel, LedColor color)
        {
            if (stream.CanWrite)
            {
                byte[] buffer = new byte[64];
                buffer[0] = 0x00;
                buffer[1] = 0xAA;

                buffer[2] = Convert.ToByte(pixel);
                buffer[3] = color.R;
                buffer[4] = color.G;
                buffer[5] = color.B;

                stream.Write(buffer, 0, buffer.Length);
            }
        }

        public void SetPixelCrossFade(int pixel, LedColor start, LedColor end, int fadeTime, bool repeat = false)
        {
            if (stream.CanWrite)
            {
                byte[] buffer = new byte[64];
                buffer[0] = 0x00;
                buffer[1] = 0x01;

                buffer[2] = Convert.ToByte(pixel);
                buffer[3] = start.R;
                buffer[4] = start.G;
                buffer[5] = start.B;
                buffer[6] = end.R;
                buffer[7] = end.G;
                buffer[8] = end.B;
                var time = System.BitConverter.GetBytes(fadeTime);
                buffer[9] = time[0];
                buffer[10] = time[1];
                buffer[11] = time[2];
                buffer[12] = time[3];
                buffer[13] = Convert.ToByte(repeat);


                stream.Write(buffer, 0, buffer.Length);
            }
        }

        public bool IsBeingHeld()
        {
            AccelData container;
            container.X = Math.Abs(accelData.X - oldAccelData.X);
            container.Y = Math.Abs(accelData.Y - oldAccelData.Y);
            container.Z = Math.Abs(accelData.Z - oldAccelData.Z);
            past.Add(container);
            if (past.Count > 20)
            {
                container.X = 0;
                container.Y = 0;
                container.Z = 0;
                for (int i = 0; i < past.Count; i++)
                {
                    container.X += past[i].X;
                    container.Y += past[i].Y;
                    container.Z += past[i].Z;
                }
                container.X = container.X / past.Count;
                container.Y = container.Y / past.Count;
                container.Z = container.Z / past.Count;
                past.RemoveAt(0);
            }
            if (container.X > .11 || container.Y > .11 || container.Z > .11)
                return true;
            else
                return false;

        }

        public void SetPattern(int id)
        {
            byte[] buffer = new byte[64];
            switch (id)
            {

                case 0:
                    buffer[0] = 0x00;
                    buffer[1] = 0x03;
                    stream.Write(buffer, 0, buffer.Length);
                    break;
                case 1:
                    buffer[0] = 0x00;
                    buffer[1] = 0x04;
                    stream.Write(buffer, 0, buffer.Length);
                    break;
                case 2:
                    buffer[0] = 0x00;
                    buffer[1] = 0x05;
                    stream.Write(buffer, 0, buffer.Length);
                    break;
                case 3:
                    buffer[0] = 0x00;
                    buffer[1] = 0x06;
                    stream.Write(buffer, 0, buffer.Length);
                    break;
                default:
                    break;
            }
        }

        public void ExportSensorData(string filename)
        {
            save = false;
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(filename + ".csv"))
            {
                file.WriteLine("X, Y, Z, Temp C, Temp F, Light, Sound, Left Button, Right Button, Slide Switch");
                foreach (var Stat in Stats)
                {
                    file.WriteLine("{0},{1},{2},{3},{4},{5},{6},{7},{8},{9}", Stat.accelData.X, Stat.accelData.Y, Stat.accelData.Z, Stat.tempC, Stat.tempF, Stat.light, Stat.sound, Stat.leftButton, Stat.rightButton, Stat.slideSwitch);
                }
            }
            save = true;
        }
    }
}
