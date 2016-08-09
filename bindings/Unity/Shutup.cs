using UnityEngine;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Shutup {
    public class Checker {
        public enum MatchType {
            Front,
            Back,
            Contain,
            Exact,
        };
        public struct Context {
            public MatchType _matchType;
            public Context(MatchType matchType) {
                _matchType = matchType;
            }
        };

        //dll API
#if UNITY_EDITOR || UNITY_ANDROID
        const string DllName = "shutup";
#elif UNITY_IPHONE
        const string DllName = "__Internal";
#else
        #error "invalid arch"
#endif
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate bool CheckerDelegate(byte *inp, int ilen, int start, int count, void *ctx);

        [DllImport (DllName)]
        private static extern unsafe void *shutup_new(byte *lang, void *a);

        [DllImport (DllName)]
        private static extern unsafe void shutup_delete(void *s);

        [DllImport (DllName)]
        private static extern unsafe void shutup_set_alias(void *s, byte *target, byte *alias);

        [DllImport (DllName)]
        private static extern unsafe void shutup_ignore_glyphs(void *s, byte *glyphs);

        [DllImport (DllName)]
        private static extern unsafe void shutup_add_word(void *s, byte *word, void *ctx);

        [DllImport (DllName)]
        private static extern unsafe Context *shutup_should_filter(void *s, byte *i, int ilen, int *start, int *count, CheckerDelegate d);

        [DllImport (DllName)]
        private static extern unsafe void *shutup_filter(void *s, byte *i, int ilen, byte *o, int *olen, byte *mask, CheckerDelegate d);

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
        List<Context> _holder = new List<Context>();
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
            byte[] b = System.Text.Encoding.UTF8.GetBytes(glyphs + "\0");
            unsafe {
                fixed (byte* g = b) {
                    shutup_ignore_glyphs(_shutter.ToPointer(), g);
                }
            }
            return this;
        }
        public Checker AddWord(string word, MatchType matchType) {
            byte[] b = System.Text.Encoding.UTF8.GetBytes(word + "\0");
            Context ctx = new Context(matchType);
            _holder.Add(ctx);
            int id = _holder.Count;
            unsafe {
                fixed (byte* w = b) {
                    //Debug.Log("AddWord: " + string.Format("{0}[{1:X2}...{2:X2}{3:X2}{4:X2}]", (ulong)_shutter.ToPointer(), w[0], w[b.Length - 2], w[b.Length - 1], w[b.Length]));
                    shutup_add_word(_shutter.ToPointer(), w, (void *)id);
                }
            }
            return this;
        }
        //禁止語句のパターンにマッチした時に、紐づけられたコンテクストを参照して、実際に禁止語句として扱うかを判断するコールバック.
        public unsafe bool ContextChecker(byte *inp, int ilen, int start, int count, void *_ctx) {
            int id = (int)_ctx;
            var ctx = _holder[id - 1];
            switch (ctx._matchType) {
            case MatchType.Front:
                if (start != 0) {
                    return false;
                }
                break;
            case MatchType.Back:
                if ((start + count) != ilen) {
                    return false;
                }
                break;
            case MatchType.Contain:
                break;
            case MatchType.Exact:
                if (start != 0 || count != ilen) {
                    return false;
                }
                break;
            }
            return true;
        }
        public string ShouldFilter(string text) {
            byte[] b = System.Text.Encoding.UTF8.GetBytes(text);
            int start = 0, count = 0;
            unsafe {
                fixed (byte* t = b) {
                    //Debug.Log("ShouldFilter: " + string.Format("{0}[{1:X2}{2:X2}]", (ulong)_shutter.ToPointer(), t[b.Length - 1], t[b.Length - 0]));
                    if (shutup_should_filter(_shutter.ToPointer(), t, b.Length, &start, &count, ContextChecker) != null) {
                        return System.Text.Encoding.UTF8.GetString(b, start, count);
                    }
                }
            }
            return null;
        }
        public string Filter(string text, string mask = null) {
            byte[] b = System.Text.Encoding.UTF8.GetBytes(text);
            byte[] b2 = (mask == null ? null : System.Text.Encoding.UTF8.GetBytes(mask + "\0"));
            byte[] outbuf = new byte[b.Length * (mask == null ? 1 : b2.Length)];
            int olen = outbuf.Length;
            unsafe {
                fixed (byte* t = b, m = b2) {
                    fixed (byte* o = outbuf) {
                        if (shutup_filter(_shutter.ToPointer(), t, b.Length, o, &olen, m, ContextChecker) == null) {
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
