using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Diagnostics;

namespace WinForms_CSExample
{
    static class Native
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string dllToLoad);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll")]
        public static extern bool FreeLibrary(IntPtr hModule);

        /// <summary>
        /// Marshals the native DLL func to a delegte
        /// </summary>
        /// <typeparam name="T">The delegate to type to</typeparam>
        /// <param name="pDll">LoadLibrary dll handle</param>
        /// <param name="fName">function name for GetProcAddress</param>
        /// <returns></returns>
        #region public static T GetFunc<T>(IntPtr pDll, string fName) where T : class
        public static T GetFunc<T>(IntPtr pDll, string fName) where T : class
        {
            IntPtr pFunc = GetFunctionIntPtr(pDll, fName);

            System.Delegate retval = Marshal.GetDelegateForFunctionPointer(pFunc, typeof(T));
            T func = retval as T;

            return func;
        }
        #endregion

        /// <summary>
        /// Does a GetProcAddress
        /// </summary>
        /// <param name="pDll">LoadLibrary dll handle</param>
        /// <param name="functionName">function name for GetProcAddress</param>
        /// <returns>pointer to the function from the dll</returns>
        #region public static IntPtr GetFunctionIntPtr(IntPtr pDll, string functionName)
        public static IntPtr GetFunctionIntPtr(IntPtr pDll, string functionName)
        {
            IntPtr pAddressOfFunctionToCall = GetProcAddress(pDll, functionName);

            // oh dear, error handling here
            if (pDll == IntPtr.Zero)
                throw new InvalidOperationException("Bubbles DLL not loaded");

            //oh dear, error handling here
            if (pAddressOfFunctionToCall == IntPtr.Zero)
                throw new InvalidOperationException(string.Format("Function '{0}' not found", functionName));

            return pAddressOfFunctionToCall;
        }
        #endregion
    }

    public class BubbleLib
    {
        public delegate void TimerTraceFunc(uint engineId, int duration);
        public delegate void GetCoordsFunc(uint engineId, uint id, ref float x, ref float y, ref float z);
        public delegate void CollisionReportFunc(uint groupId, uint engineId, IntPtr arrayPtr, uint size);
 
        public delegate bool Init();
        public delegate void UnInit();
        public delegate uint AddEngine();
        //public delegate void SetEngineTimerTrace(uint engineId, TimerTraceFunc timerTraceFunc);
        public delegate uint GetEngineCount();
        public delegate uint AddEngineGroup(uint engineId);
        public delegate uint AddEngineToGroup(uint engineGroupId, uint engineId);
        public delegate uint GetGroupCount();
        public delegate bool AddBubble(uint engineId, uint id, float radius, GetCoordsFunc getCoordsFunc);
        public delegate uint GetBubbleCount(uint engineId);
        public delegate void StartEngine(uint engineId, CollisionReportFunc reportFunc, uint intervalMS);
        public delegate void PauseEngine(uint engineId, bool pause);
        public delegate void PauseGroup(uint groupId, bool pause);

        public const string DllName = "Bubbles.dll";

        public const string InitName = "Init";
        public const string UnInitName = "UnInit";
        public const string AddEngineName = "AddEngine";
        //public const string SetEngineTimerTraceName = "SetEngineTimerTrace";
        public const string GetEngineCountName = "GetEngineCount";
        public const string AddEngineGroupName = "AddEngineGroup";
        public const string AddEngineToGroupName = "AddEngineToGroup";
        public const string GetGroupCountName = "GetGroupCount";
        public const string AddBubbleName = "AddBubble";
        public const string GetBubbleCountName = "GetBubbleCount";
        public const string StartEngineName = "StartEngine";
        public const string PauseEngineName = "PauseEngine";
        public const string PauseGroupName = "PauseGroup";
    }

    public class Bubbles : IDisposable
    {
        #region public enum cAxisSplitterAXIS : int
        public enum cAxisSplitterAXIS : int
        {
            X = 0, Y, Z,
            NoOfDimensions
        };
        #endregion

        [StructLayout(LayoutKind.Sequential), Serializable]
        #region public struct DistanceUnit
        public struct Trilateration
        {
            public cAxisSplitterAXIS axis;
            public float abs_dist; //absolute distance from id
            public float rel_coord; //x, y or z of id
            public uint id;
        };
        #endregion

        [StructLayout(LayoutKind.Sequential), Serializable]
        #region public struct Trilateration
        public struct CollisionResult
        {
            public uint mCenterID;
            public Trilateration mDistanceUnit1;// = new DistanceUnit[3];
            public Trilateration mDistanceUnit2;
            public Trilateration mDistanceUnit3;
        };
        #endregion

        private IntPtr mDll = Native.LoadLibrary(BubbleLib.DllName);
        public void Dispose()
        {
            UnInit();
            bool result = Native.FreeLibrary(mDll);
        }

        public Bubbles()
        {
            Init();
        }
        
        /// <summary>
        /// Turn the ref into a typed List 
        /// http://stackoverflow.com/questions/2402978/get-an-array-of-structures-from-native-dll-to-c-sharp-application
        /// </summary>
        #region private static List<Trilateration> GetTrilaterations(ref IntPtr arrayPtr, uint size)
        private static List<CollisionResult> GetTrilaterations(ref IntPtr arrayPtr, uint size)
        {
            var arrayValue = arrayPtr;
            var list = new List<CollisionResult>();

            if (size == 0)
                return list;

            var tableEntrySize = Marshal.SizeOf(typeof(CollisionResult));
            for (var i = 0; i < size; i++)
            {
                var cur = (CollisionResult)Marshal.PtrToStructure(arrayValue, typeof(CollisionResult));
                list.Add(cur);
                arrayValue = new IntPtr(arrayValue.ToInt32() + tableEntrySize);
            }
            return list;
        }
        #endregion

        /// <summary>
        /// Used to build a compound key of EngineId and BubbleId
        /// </summary>
        public struct EngineBubbleKey
        {
            public uint EngineId;
            public uint BubbleId;
        }

        public static EngineBubbleKey MakeKey(uint engineId, uint id) { return new EngineBubbleKey() { EngineId = engineId, BubbleId = id }; } //(ulong)engineId << 32 | id; }

        #region private BubbleLib.Init Init
        private static BubbleLib.Init mInit;
        private BubbleLib.Init Init
            { get { return mInit ?? (mInit = Native.GetFunc<BubbleLib.Init>(mDll, BubbleLib.InitName)); } }
        #endregion

        #region private BubbleLib.UnInit UnInit
        private static BubbleLib.UnInit mUnInit;
        private BubbleLib.UnInit UnInit
            { get { return mUnInit ?? (mUnInit = Native.GetFunc<BubbleLib.UnInit>(mDll, BubbleLib.UnInitName)); } }
        #endregion

        #region public BubbleLib.AddEngine AddEngine
        private static BubbleLib.AddEngine mAddEngine;
        public BubbleLib.AddEngine AddEngine
            { get { return mAddEngine ?? (mAddEngine = Native.GetFunc<BubbleLib.AddEngine>(mDll, BubbleLib.AddEngineName)); } }
        #endregion

        #region public BubbleLib.SetEngineTimerTrace SetEngineTimerTrace
        //private static BubbleLib.SetEngineTimerTrace mSetEngineTimerTrace;
        //public BubbleLib.SetEngineTimerTrace SetEngineTimerTrace
        //    { get { return mSetEngineTimerTrace ?? (mSetEngineTimerTrace = Native.GetFunc<BubbleLib.SetEngineTimerTrace>(mDll, BubbleLib.SetEngineTimerTraceName)); } }
        #endregion
        
        #region public BubbleLib.GetEngineCount GetEngineCount
        private static BubbleLib.GetEngineCount mGetEngineCount;
        public BubbleLib.GetEngineCount GetEngineCount
            { get { return mGetEngineCount ?? (mGetEngineCount = Native.GetFunc<BubbleLib.GetEngineCount>(mDll, BubbleLib.GetEngineCountName)); } }
        #endregion

        #region public BubbleLib.AddEngineGroup AddEngineGroup
        private static BubbleLib.AddEngineGroup mAddEngineGroup;
        public BubbleLib.AddEngineGroup AddEngineGroup
        { get { return mAddEngineGroup ?? (mAddEngineGroup = Native.GetFunc<BubbleLib.AddEngineGroup>(mDll, BubbleLib.AddEngineGroupName)); } }
        #endregion

        #region public BubbleLib.AddEngineToGroup AddEngineToGroup
        private static BubbleLib.AddEngineToGroup mAddEngineToGroup;
        public BubbleLib.AddEngineToGroup AddEngineToGroup
        { get { return mAddEngineToGroup ?? (mAddEngineToGroup = Native.GetFunc<BubbleLib.AddEngineToGroup>(mDll, BubbleLib.AddEngineToGroupName)); } }
        #endregion

        #region public BubbleLib.GetGroupCount GetGroupCount
        private static BubbleLib.GetGroupCount mGetGroupCount;
        public BubbleLib.GetGroupCount GetGroupCount
        { get { return mGetGroupCount ?? (mGetGroupCount = Native.GetFunc<BubbleLib.GetGroupCount>(mDll, BubbleLib.GetGroupCountName)); } }
        #endregion

        public delegate void GetCoords(uint engineId, ref float x, ref float y, ref float z);
        #region public bool AddBubble(uint engineId, uint id, float radius, GetCoords getCoordsFunc)
        public bool AddBubble(uint engineId, uint id, float radius, GetCoords getCoordsFunc)
        {
            ApplicationGetCoordsCallback.Add(MakeKey(engineId, id), getCoordsFunc);
            return AddBubbleNative(engineId, id, radius, NativeGetCoordsCallback);
        }
        #endregion
        /* the alternative to a dictionary of delegates seems to be this horrible event hack http://msdn.microsoft.com/en-us/library/z4ka55h8.aspx */
        private static Dictionary<EngineBubbleKey, GetCoords> ApplicationGetCoordsCallback = new Dictionary<EngineBubbleKey, GetCoords>();
        private static BubbleLib.GetCoordsFunc NativeGetCoordsCallback = new BubbleLib.GetCoordsFunc(NativeGetCoords);

        #region private static void NativeGetCoords(uint engineId, uint id, ref float x, ref float y, ref float z)
        private static void NativeGetCoords(uint engineId, uint id, ref float x, ref float y, ref float z)
        {            
            ApplicationGetCoordsCallback[MakeKey(engineId, id)] ( id, ref x, ref y, ref z );
        }
        #endregion
        #region private BubbleLib.AddBubble AddBubbleNative
        private static BubbleLib.AddBubble mAddBubble;
        private BubbleLib.AddBubble AddBubbleNative
            { get { return mAddBubble ?? (mAddBubble = Native.GetFunc<BubbleLib.AddBubble>(mDll, BubbleLib.AddBubbleName)); } }
        #endregion
        #region public BubbleLib.GetBubbleCount GetBubbleCount
        private static BubbleLib.GetBubbleCount mGetBubbleCount;
        public BubbleLib.GetBubbleCount GetBubbleCount
            { get { return mGetBubbleCount ?? (mGetBubbleCount = Native.GetFunc<BubbleLib.GetBubbleCount>(mDll, BubbleLib.GetBubbleCountName)); } }
        #endregion

        public delegate void CollisionCallback(List<CollisionResult> bangs);
        #region public void StartEngine(uint engineId, CollisionCallback appCallback)
        public void StartEngine(uint engineId, CollisionCallback appCallback, uint intervalMS)
        {
            ApplicationCollisionCallback.Add(engineId, appCallback);
            StartEngineNative(engineId, NativeCollisionReportCallback, intervalMS);
        }
        #endregion

        private static Dictionary<uint /*engineId*/, CollisionCallback> ApplicationCollisionCallback = new Dictionary<uint, CollisionCallback>();
        #region private BubPactLib.StartEngine StartEngineNative
        private static BubbleLib.StartEngine mStartEngineNative;
        private BubbleLib.StartEngine StartEngineNative
            { get { return mStartEngineNative ?? (mStartEngineNative = Native.GetFunc<BubbleLib.StartEngine>(mDll, BubbleLib.StartEngineName)); } }
        #endregion

        private static BubbleLib.PauseEngine mPauseEngine;
        /// <summary>
        /// Pauses the specified engine
        /// BubbleUtil.PauseAll also available to pause everything
        /// </summary>
        public BubbleLib.PauseEngine PauseEngine
            { get { return mPauseEngine ?? (mPauseEngine = Native.GetFunc<BubbleLib.PauseEngine>(mDll, BubbleLib.PauseEngineName)); } }

        private static BubbleLib.PauseGroup mPauseGroup;
        /// <summary>
        /// Pauses the specified group of engines
        /// BubbleUtil.PauseAll also available to pause everything
        public BubbleLib.PauseGroup PauseGroup
        { get { return mPauseGroup ?? (mPauseGroup = Native.GetFunc<BubbleLib.PauseGroup>(mDll, BubbleLib.PauseGroupName)); } }

        private static BubbleLib.CollisionReportFunc NativeCollisionReportCallback = new BubbleLib.CollisionReportFunc(NativeCollisionReport);
        #region private static void NativeCollisionReport(uint engineId, IntPtr arrayPtr, uint size)
        private static void NativeCollisionReport(uint groupId, uint engineId, IntPtr arrayPtr, uint size)
        {
            var bangs = GetTrilaterations(ref arrayPtr, size);
            ApplicationCollisionCallback[engineId] ( bangs );
        }
        #endregion
    }

    public class ShowBangEventArgs : EventArgs
    {
        public Bubbles.CollisionResult Bang { get; private set; }
        public BubbleUtil Util { get; set; }
        public ShowBangEventArgs(BubbleUtil util, Bubbles.CollisionResult bang)
        {
            Bang = bang;
            Util = util;
        }
    }

    public class BubbleUtil
    {
        public event EventHandler<ShowBangEventArgs> ShowBang;
        protected void OnShowBang(ShowBangEventArgs args)
        {
            if (ShowBang != null)
                ShowBang(this, args);
        }
        private System.Windows.Forms.Timer CollisionDrainerTimer;

        #region public BubbleUtil()
        public BubbleUtil()
        {
            CollisionDrainerTimer = new System.Windows.Forms.Timer() { Interval = 100 };
            CollisionDrainerTimer.Tick += new EventHandler(CollisionDrainerTimer_Tick);
            CollisionDrainerTimer.Enabled = true;
        }
        #endregion

        #region public void StartEngine(BubPact bubPact, uint engineId, ShowBang showBangCallback)
        public void StartEngine(Bubbles bubPact, uint engineId, uint intervalMS)
        {
            bubPact.StartEngine(engineId, delegate(List<Bubbles.CollisionResult> list)
                { BubbleUtil.Bangs(this, engineId, list); }, intervalMS);
        }
        #endregion

        #region public void PauseAll(Bubbles bubPact, bool pause)
        public void PauseAll(Bubbles bubPact, bool pause)
        {
            uint groupCount = bubPact.GetGroupCount();
            for (uint i=0; i<groupCount; i++)
                bubPact.PauseGroup(i, pause);

            CollisionDrainerTimer.Enabled = (pause == false);
        }
        #endregion

        private ConcurrentQueue<Bubbles.CollisionResult> bangQueue;
        public ConcurrentQueue<Bubbles.CollisionResult> BangQueue { get { return bangQueue ?? (bangQueue = new ConcurrentQueue<Bubbles.CollisionResult>()); } }

        private ConcurrentDictionary<Bubbles.EngineBubbleKey, bool> hitLookup;
        public ConcurrentDictionary<Bubbles.EngineBubbleKey, bool> HitLookup { get { return hitLookup ?? (hitLookup = new ConcurrentDictionary<Bubbles.EngineBubbleKey, bool>()); } }

        #region public bool IsHit(uint engineId, uint id)
        public bool IsHit(uint engineId, uint id)
        {
            bool ret;
            if (HitLookup.TryGetValue(Bubbles.MakeKey(engineId, id), out ret))
                return ret;
            return false;
        }
        #endregion

        #region public void SetHit(uint engineId, uint id, bool value)
        public void SetHit(uint engineId, uint id, bool value)
        {
            Bubbles.EngineBubbleKey key = Bubbles.MakeKey(engineId, id);
            bool getit;
            if (HitLookup.TryGetValue(key, out getit) == false)
                HitLookup.TryAdd(key, value);
            else
                HitLookup[key] = value;
        }
        #endregion

        #region private void CollisionDrainerTimer_Tick(object sender, EventArgs e)
        private void CollisionDrainerTimer_Tick(object sender, EventArgs e)
        {
            Bubbles.CollisionResult result;
            while (BangQueue.TryDequeue(out result))
                OnShowBang(new ShowBangEventArgs(this, result));
        }
        #endregion

        /// <summary>
        /// The collision engine callback where it tells us the collisions. This is on a different thread so we put the results
        /// in a concurrent queue
        /// 
        /// Items /no longer/ in a hit state are marked as such in the HitLookup
        /// </summary>
        /// <param name="bangs"></param>
        #region public static void Bangs(BubPactUtil me, List<BubPact.Trilateration> bangs)
        public static void Bangs(BubbleUtil me, uint engineId, List<Bubbles.CollisionResult> bangs)
        {
            // put the collisions on the thread safe queue
            foreach (var bang in bangs)
                me.BangQueue.Enqueue(bang);

            // mark all those items that are no longer hit
            foreach (var key in me.HitLookup.Keys.
                                    //AsParallel().
                                    Where(h => engineId == h.EngineId && bangs.Select(t => t.mCenterID).Contains(h.BubbleId) == false))
            {
                me.HitLookup[key] = false;
            }

        }
        #endregion

        //public static BubbleLib.TimerTraceFunc NativeTimerTraceCallback = new BubbleLib.TimerTraceFunc(TimerTraceFunc);
        //#region public void TimerTraceFunc(uint engineId, int duration)
        //private static void TimerTraceFunc(uint engineId, int duration)
        //{
        //    Trace.WriteLine(string.Format("Engine = {0}  Duration = {1} ms", engineId, duration));
        //}
        //#endregion
    }
}
