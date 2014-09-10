using System;
using System.Drawing;
using System.Windows.Forms;

namespace WinForms_CSExample
{
    internal class BangButton : Button
    {
        private BubbleUtil Util;
        public uint EngineId { get; set; }
        public uint Id { get; set; }

        public BangButton(BubbleUtil util, uint engineId=0) { this.SetStyle(ControlStyles.UserPaint, true); Util = util; EngineId = engineId; }

        public bool IsHit
        {
            get { return Util.IsHit(EngineId, Id); }
            set { Util.SetHit(EngineId, Id, value); }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            bool isHit = IsHit;
            if (isHit)
            {
                using (SolidBrush redBrush = new SolidBrush(Color.Red))
                    e.Graphics.FillRectangle(redBrush, ClientRectangle);

                using (SolidBrush blackBrush = new SolidBrush(Color.Black))
                    e.Graphics.DrawString(string.Format("OUCH!{0}{1}", Environment.NewLine, this.Text), this.Font, blackBrush, new Point());

                return;
            }
            base.OnPaint(e);
        }
    }
}
