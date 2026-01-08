using System;
using System.Collections.Generic;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class ArrayUtils
    {
        public static void scaleInPlace(double[] array, double factor)
        {
            for (int i = 0; i < array.Length; i++)
            {
                array[i] *= factor;
            }
        }

        public static double getAverage(double[] array)
        {
            double sum = 0;
            for (int i = 0; i < array.Length; ++i)
            {
                sum += array[i];
            }
            return sum / array.Length;
        }

        public static System.Numerics.Complex getAverage(System.Numerics.Complex[] array)
        {
            System.Numerics.Complex sum = 0;
            for (int i = 0; i < array.Length; ++i)
            {
                sum += array[i];
            }
            return sum / array.Length;
        }

        public static void offsetInPlace(double[] array, double offset)
        {
            for (int i = 0; i < array.Length; i++)
            {
                array[i] += offset;
            }
        }

        public static void offsetInPlace(System.Numerics.Complex[] array, System.Numerics.Complex offset)
        {
            for (int i = 0; i < array.Length; i++)
            {
                array[i] += offset;
            }
        }
    }
}
