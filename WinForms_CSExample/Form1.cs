using System;
using System.Collections.Generic;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using System.Diagnostics;

namespace WinForms_CSExample
{
    public partial class Form1 : Form
    {
        private System.Windows.Forms.Timer ButtonMoveTimer = new System.Windows.Forms.Timer();
        private List<BangButton> ButtonList = new List<BangButton>();
        private object NewButtonLock = new object();
        private uint NextButtonID;

        private Bubbles myBubbles = new Bubbles();
        private BubbleUtil Util = new BubbleUtil();

        private uint EngineGroupId;
        private uint EngineId1;
        private uint EngineId2;

        private uint LastEngineAddedTo;

        /// <summary>
        /// Returns either the first or second engine Id, alternating between the two
        /// </summary>
        private uint EngineIdAddHelper
        {
            get
            {
                return LastEngineAddedTo == EngineId1
                    ? (LastEngineAddedTo = EngineId2) : (LastEngineAddedTo = EngineId1);
            }
        }

        private static System.Media.SoundPlayer CrashPlayer = new System.Media.SoundPlayer("crash.wav");

        public Form1()
        {
            InitializeComponent();

            // Sequence of events with the bubble engine is somewhat important when operating with groups
            // - Add engines first
            // - Then map engines group associations
            // - THEN add the impact bubbles to respective engines
            EngineId1 = myBubbles.AddEngine();
            //myBubbles.SetEngineTimerTrace(EngineId1, BubbleUtil.NativeTimerTraceCallback);
            EngineId2 = myBubbles.AddEngine();
            //myBubbles.SetEngineTimerTrace(EngineId2, BubbleUtil.NativeTimerTraceCallback);

            EngineGroupId = myBubbles.AddEngineGroup(EngineId1);
            myBubbles.AddEngineToGroup(EngineGroupId, EngineId2);

            MakeNewBubbleButton();

            Util.ShowBang += new EventHandler<ShowBangEventArgs>(Util_ShowBang);

            // Both engines are associated with the same group. We will add new bubbles to either engine1 or 
            // engine2, alternating between the two. Then the collision detection work is split with each engine 
            // reporting on its bubbles, but comparing its workload over all bubbles in its engines group.
            Util.StartEngine(myBubbles, EngineId1, 200);
            Util.StartEngine(myBubbles, EngineId2, 200);

            bool paused = false;
            this.Deactivate += new EventHandler(delegate(object sender, EventArgs e)
                {
                    if (paused) return;
                    paused = true;
                    Util.PauseAll(myBubbles, true);
                    ButtonMoveTimer.Stop();
                });

            this.Activated += new EventHandler(delegate(object sender, EventArgs e)
                {
                    if (paused == false) return;
                    paused = false;
                    ButtonMoveTimer.Start();
                    Util.PauseAll(myBubbles, false);
                });

            ButtonMoveTimer.Interval = 100;
            ButtonMoveTimer.Tick += new EventHandler(ButtomMoveTimer_Tick);
            ButtonMoveTimer.Enabled = true;

            System.Windows.Forms.Timer invalidateCheck = new System.Windows.Forms.Timer() { Interval = 200, Enabled = true };
            invalidateCheck.Tick += new EventHandler(delegate(object sender, EventArgs e) 
                {
                    foreach (BangButton but in ButtonList)
                        but.Invalidate();
                });
        }

        void Util_ShowBang(object sender, ShowBangEventArgs e)
        {
            BangButton found = ButtonList.Find(but => but.Id == (int)e.Bang.mCenterID);
            if (e.Util.IsHit(found.EngineId, e.Bang.mCenterID)) return;

            // big breath then CrashPlayer.Play()
            Thread.SpinWait(20);
            CrashPlayer.Stop();
            Thread.SpinWait(20); 
            CrashPlayer.Play(); // hog
            Thread.SpinWait(20);

            e.Util.SetHit(found.EngineId, e.Bang.mCenterID, true);
            e.Util.SetHit(found.EngineId, e.Bang.mDistanceUnit1.id, true);
        }

        /// <summary>
        /// Engine callback so the engine can ask the bubbles location
        /// </summary>
        /// <param name="id">Id of the Bubble</param>
        /// <param name="x">ref to recieve the x coordinate</param>
        /// <param name="y">ref to recieve the y coordinate</param>
        /// <param name="z">ref to recieve the z coordinate</param>
        private void GetCoords(uint id, ref float x, ref float y, ref float z)
        {
            Button found = ButtonList.Find(but => but.Id == (int)id);
            if (found == null)
                return;

            x = found.Location.X;
            y = found.Location.Y;
            z = 0.0f;
        }

        private void ButtomMoveTimer_Tick(object sender, EventArgs e)
        {
            foreach (Button button in ButtonList)
            {
                // Move the button a bit
                int newX, newY;
                Point vector = (Point)button.Tag;
                newX = button.Location.X + vector.X;
                newY = button.Location.Y + vector.Y;

                // Ensure the button stays on the form
                CheckBounds(this.ClientSize, ref newX, ref newY);
                button.Location = new Point(newX, newY);
            }
        }

        private static void CheckBounds(Size boundSize, ref int newX, ref int newY)
        {
            if (newX > boundSize.Width) newX = 0;
            if (newX < 0) newX = boundSize.Width;
            if (newY > boundSize.Height) newY = 0;
            if (newY < 0) newY = boundSize.Height;
        }

        private static int randomVectorIndex = 0;
        private static int[] randomVector1 = { -1, 0, 1, 2, 1, -1, -2, 0, -2 };
        private static int[] randomVector2 = { -2, 0, 2, -1, 1, -1, -1, -2, -2 };
        private static bool randomFlipFlop = false;
        private static int GetRandomish()
        {
            int ans = randomFlipFlop ?
                randomVector1[randomVectorIndex] : randomVector2[randomVectorIndex];

            randomFlipFlop = !randomFlipFlop;
            randomVectorIndex++;
            if (randomVectorIndex == 9)
                randomVectorIndex = 0;

            return ans;
        }
        
        private BangButton MakeNewBubbleButtonLocked()
        {
            ++NextButtonID;

            const int height = 30;
            BangButton button1 = new BangButton(Util);
            button1.Id = NextButtonID;

            button1.Location = new System.Drawing.Point(1, 1);
            button1.Name = string.Format("{0}", NextButtonID);
            button1.Font = new Font(FontFamily.GenericSansSerif, 8.0f);
            button1.Size = new System.Drawing.Size(height, height);
            button1.TabIndex = (int)NextButtonID;
            button1.Text = button1.Name;
            button1.Visible = true;
            button1.Tag = new Point(GetRandomish(), GetRandomish()); // this is its direction
            button1.Click += new EventHandler(button1_Click);

            bool result;

            /* while */
            {
                uint allocatedEngineId = EngineIdAddHelper;
                button1.EngineId = allocatedEngineId;
                result = myBubbles.AddBubble(allocatedEngineId, NextButtonID, (float)height, GetCoords);
                if (result)
                {
                    Controls.Add(button1);
                    ButtonList.Add(button1);
                }
            } while (result == false) ;

            string text = string.Format("group count={0}, engine count={1}, bubble count1={2}, bubble count2={3}",
                myBubbles.GetGroupCount(),
                myBubbles.GetEngineCount().ToString(),
                myBubbles.GetBubbleCount(EngineId1).ToString(),
                myBubbles.GetBubbleCount(EngineId2).ToString());

            Text = text;
            return button1;
        }

        private BangButton MakeNewBubbleButton()
        {
            lock (NewButtonLock)
            {
                return MakeNewBubbleButtonLocked();
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            MakeNewBubbleButton();
        }
    }
}
