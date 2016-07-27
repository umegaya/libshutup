using UnityEngine;
using System.Runtime.InteropServices;

namespace Shutup {
    public class Checker {
        //dll API
#if UNITY_EDITOR || UNITY_ANDROID
        const string DllName = "shutup";
#elif UNITY_IPHONE
        const string DllName = "__Internal";
#else
        #error "invalid arch"
#endif

        [DllImport (DllName)]
        private static extern unsafe void *shutup_new(byte *lang, void *a);

        [DllImport (DllName)]
        private static extern unsafe void shutup_delete(void *s);

        [DllImport (DllName)]
        private static extern unsafe void shutup_set_alias(void *s, byte *target, byte *alias);

        [DllImport (DllName)]
        private static extern unsafe void shutup_ignore_glyphs(void *s, byte *glyphs);

        [DllImport (DllName)]
        private static extern unsafe void shutup_add_word(void *s, byte *word);

        [DllImport (DllName)]
        private static extern unsafe void *shutup_should_filter(void *s, byte *i, int ilen, byte *o, int *olen);

        [DllImport (DllName)]
        private static extern unsafe void *shutup_filter(void *s, byte *i, int ilen, byte *o, int *olen, byte *mask);

        [DllImport (DllName)]
        private static extern void shutup_set_logger(LoggerDelegate logger);

#if UNITY_EDITOR
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void LoggerDelegate(string str);
        static void Logger(string str) {
            Debug.Log("shutup: " + str);
        }

        public static void InitLogger() {
            shutup_set_logger(Logger);
        }
#endif
        
        System.IntPtr _shutter;
        public void Init(string lang) {
            byte[] b = System.Text.Encoding.UTF8.GetBytes(lang);
            unsafe {
                fixed (byte *l = b) {
                    _shutter = new System.IntPtr(shutup_new(l, null));
                }
            }
        }
        public void Fin() {
            unsafe {
                if (_shutter != null) {
                    shutup_delete(_shutter.ToPointer());
                }
            }
        }
        public Checker SetAlias(string target, string alias) {
            //to add null byte at the last of converted byte array
            byte[] b1 = System.Text.Encoding.UTF8.GetBytes(target + "\0");
            byte[] b2 = System.Text.Encoding.UTF8.GetBytes(alias + "\0");
            unsafe {
                fixed (byte* t = b1, a = b2) {
                    shutup_set_alias(_shutter.ToPointer(), t, a);
                }
            }
            return this;
        }
        public Checker IgnoreGlyphs(string glyphs) {
            //to add null byte at the last of converted byte array
            byte[] b = System.Text.Encoding.UTF8.GetBytes(glyphs + "\0");
            unsafe {
                fixed (byte* g = b) {
                    shutup_ignore_glyphs(_shutter.ToPointer(), g);
                }
            }
            return this;
        }
        public Checker AddWord(string word) {
            //to add null byte at the last of converted byte array
            byte[] b = System.Text.Encoding.UTF8.GetBytes(word + "\0");
            unsafe {
                fixed (byte* w = b) {
                    //Debug.Log("AddWord: " + string.Format("{0}[{1:X2}...{2:X2}{3:X2}{4:X2}]", (ulong)_shutter.ToPointer(), w[0], w[b.Length - 2], w[b.Length - 1], w[b.Length]));
                    shutup_add_word(_shutter.ToPointer(), w);
                }
            }
            return this;
        }
        public string ShouldFilter(string text) {
            //b is exactly treated as byte array (with length), so no need to add \0
            byte[] b = System.Text.Encoding.UTF8.GetBytes(text);
            byte[] outbuf = new byte[b.Length];
            int olen = outbuf.Length;
            unsafe {
                fixed (byte* t = b) {
                    fixed (byte* o = outbuf) {
                        //Debug.Log("ShouldFilter: " + string.Format("{0}[{1:X2}{2:X2}]", (ulong)_shutter.ToPointer(), t[b.Length - 1], t[b.Length - 0]));
                        if (shutup_should_filter(_shutter.ToPointer(), t, b.Length, o, &olen) == null) {
                            if (olen < 0) {
                                Debug.Log("ShouldFilter fails: " + olen);
                            }
                            return null;
                        }
                    }
                }
            }
            return System.Text.Encoding.UTF8.GetString(outbuf, 0, olen);
        }
        public string Filter(string text, string mask = null) {
            //b is exactly treated as byte array (with length), so no need to add \0
            byte[] b = System.Text.Encoding.UTF8.GetBytes(text);
            byte[] b2 = (mask == null ? null : System.Text.Encoding.UTF8.GetBytes(mask + "\0"));
            byte[] outbuf = new byte[b.Length * (mask == null ? 1 : b2.Length)];
            int olen = outbuf.Length;
            unsafe {
                fixed (byte* t = b, m = b2) {
                    fixed (byte* o = outbuf) {
                        if (shutup_filter(_shutter.ToPointer(), t, b.Length, o, &olen, m) == null) {
                            Debug.Log("Filter fails " + olen);
                            return text;
                        }
                    }
                }
            }
            return System.Text.Encoding.UTF8.GetString(outbuf, 0, olen);
        }
    }
}
