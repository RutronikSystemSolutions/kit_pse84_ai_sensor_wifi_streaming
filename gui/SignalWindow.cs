using System;
using System.Collections.Generic;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class SignalWindow
    {
        public enum Type
        {
            TypeBlackmanHarris,
            None
        }

        private double[] windowSignal;
        private double sum; // Useful for dBFS normalization

        public SignalWindow(Type type, int len)
        {
            windowSignal = new double[len];
            switch (type)
            {
                case Type.TypeBlackmanHarris:
                    {
                        double a0 = 0.35875;
                        double a1 = 0.48829;
                        double a2 = 0.14128;
                        double a3 = 0.01168;

                        for (int i = 0; i < len; ++i)
                        {
                            double tmp = a0
                                - (a1 * Math.Cos((2.0 * Math.PI * i) / (len - 1)))
                                + (a2 * Math.Cos((4.0 * Math.PI * i) / (len - 1)))
                                - (a3 * Math.Cos((6.0 * Math.PI * i) / (len - 1)));
                            windowSignal[i] = tmp;
                            sum += tmp;
                        }
                        break;
                    }
                case Type.None:
                    {
                        // Initialized with 1
                        for (int i = 0; i < len; ++i) windowSignal[i] = 1;
                        sum = len;
                        break;
                    }
            }
        }

        public double getSum()
        {
            return sum;
        }

        public void applyInPlace(double[] signal)
        {
            if (signal.Length != windowSignal.Length)
            {
                throw new Exception("Signal must have the same length as window");
            }

            for (int i = 0; i < signal.Length; ++i)
            {
                signal[i] = signal[i] * windowSignal[i];
            }
        }

        public void applyInPlaceComplex(System.Numerics.Complex[] signal)
        {
            if (signal.Length != windowSignal.Length)
            {
                throw new Exception("Signal must have the same length as window");
            }

            for (int i = 0; i < signal.Length; ++i)
            {
                signal[i] = signal[i] * windowSignal[i];
            }
        }
    }
}
