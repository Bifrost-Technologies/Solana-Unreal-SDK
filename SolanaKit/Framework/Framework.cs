/*
 *  Unreal Engine .NET 6 Integration
 *  Copyright (c) 2024 Bifrost Inc.
 *  Author: Nathan Martell
 *  Forked from: Stanislav Denisov's UnrealCLR
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;

namespace UnrealEngine.Framework {
 
    internal static class ArrayPool {
		[ThreadStatic]
		private static byte[] stringBuffer;

		public static byte[] GetStringBuffer() {
			if (stringBuffer == null)
				stringBuffer = GC.AllocateUninitializedArray<byte>(8192, pinned: true);

			return stringBuffer;
		}
	}

	internal static class Collector {
		[ThreadStatic]
		private static List<object> references;

		public static IntPtr GetFunctionPointer(Delegate reference) {
			if (references == null)
				references = new();

			references.Add(reference);

			return Marshal.GetFunctionPointerForDelegate(reference);
		}
	}

	internal static class Extensions {
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static T GetOrAdd<S, T>(this IDictionary<S, T> dictionary, S key, Func<T> valueCreator) => dictionary.TryGetValue(key, out var value) ? value : dictionary[key] = valueCreator();

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static byte[] StringToBytes(this string value) {
			if (value != null)
				return Encoding.UTF8.GetBytes(value);

			return null;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static string BytesToString(this byte[] buffer) {
			int end;

			for (end = 0; end < buffer.Length && buffer[end] != 0; end++);

			unsafe {
				fixed (byte* pinnedBuffer = buffer) {
					return new((sbyte*)pinnedBuffer, 0, end);
				}
			}
		}
	}

	internal static class Tables {
		// Table for fast conversion from the color to a linear color
		internal static readonly float[] Color = new float[256] {
			0.0f,
			0.000303526983548838f, 0.000607053967097675f, 0.000910580950646512f, 0.00121410793419535f, 0.00151763491774419f,
			0.00182116190129302f, 0.00212468888484186f, 0.0024282158683907f, 0.00273174285193954f, 0.00303526983548838f,
			0.00334653564113713f, 0.00367650719436314f, 0.00402471688178252f, 0.00439144189356217f, 0.00477695332960869f,
			0.005181516543916f, 0.00560539145834456f, 0.00604883284946662f, 0.00651209061157708f, 0.00699540999852809f,
			0.00749903184667767f, 0.00802319278093555f, 0.0085681254056307f, 0.00913405848170623f, 0.00972121709156193f,
			0.0103298227927056f, 0.0109600937612386f, 0.0116122449260844f, 0.012286488094766f, 0.0129830320714536f,
			0.0137020827679224f, 0.0144438433080002f, 0.0152085141260192f, 0.0159962930597398f, 0.0168073754381669f,
			0.0176419541646397f, 0.0185002197955389f, 0.0193823606149269f, 0.0202885627054049f, 0.0212190100154473f,
			0.0221738844234532f, 0.02315336579873f, 0.0241576320596103f, 0.0251868592288862f, 0.0262412214867272f,
			0.0273208912212394f, 0.0284260390768075f, 0.0295568340003534f, 0.0307134432856324f, 0.0318960326156814f,
			0.0331047661035236f, 0.0343398063312275f, 0.0356013143874111f, 0.0368894499032755f, 0.0382043710872463f,
			0.0395462347582974f, 0.0409151963780232f, 0.0423114100815264f, 0.0437350287071788f, 0.0451862038253117f,
			0.0466650857658898f, 0.0481718236452158f, 0.049706565391714f, 0.0512694577708345f, 0.0528606464091205f,
			0.0544802758174765f, 0.0561284894136735f, 0.0578054295441256f, 0.0595112375049707f, 0.0612460535624849f,
			0.0630100169728596f, 0.0648032660013696f, 0.0666259379409563f, 0.0684781691302512f, 0.070360094971063f,
			0.0722718499453493f, 0.0742135676316953f, 0.0761853807213167f, 0.0781874210336082f, 0.0802198195312533f,
			0.0822827063349132f, 0.0843762107375113f, 0.0865004612181274f, 0.0886555854555171f, 0.0908417103412699f,
			0.0930589619926197f, 0.0953074657649191f, 0.0975873462637915f, 0.0998987273569704f, 0.102241732185838f,
			0.104616483176675f, 0.107023102051626f, 0.109461709839399f, 0.1119324268857f, 0.114435372863418f,
			0.116970666782559f, 0.119538426999953f, 0.122138771228724f, 0.124771816547542f, 0.127437679409664f,
			0.130136475651761f, 0.132868320502552f, 0.135633328591233f, 0.138431613955729f, 0.141263290050755f,
			0.144128469755705f, 0.147027265382362f, 0.149959788682454f, 0.152926150855031f, 0.155926462553701f,
			0.158960833893705f, 0.162029374458845f, 0.16513219330827f, 0.168269398983119f, 0.171441099513036f,
			0.174647402422543f, 0.17788841473729f, 0.181164242990184f, 0.184474993227387f, 0.187820771014205f,
			0.191201681440861f, 0.194617829128147f, 0.198069318232982f, 0.201556252453853f, 0.205078735036156f,
			0.208636868777438f, 0.212230756032542f, 0.215860498718652f, 0.219526198320249f, 0.223227955893977f,
			0.226965872073417f, 0.23074004707378f, 0.23455058069651f, 0.238397572333811f, 0.242281120973093f,
			0.246201325201334f, 0.250158283209375f, 0.254152092796134f, 0.258182851372752f, 0.262250655966664f,
			0.266355603225604f, 0.270497789421545f, 0.274677310454565f, 0.278894261856656f, 0.283148738795466f,
			0.287440836077983f, 0.291770648154158f, 0.296138269120463f, 0.300543792723403f, 0.304987312362961f,
			0.309468921095997f, 0.313988711639584f, 0.3185467763743f, 0.323143207347467f, 0.32777809627633f,
			0.332451534551205f, 0.337163613238559f, 0.341914423084057f, 0.346704054515559f, 0.351532597646068f,
			0.356400142276637f, 0.361306777899234f, 0.36625259369956f, 0.371237678559833f, 0.376262121061519f,
			0.381326009488037f, 0.386429431827418f, 0.39157247577492f, 0.396755228735618f, 0.401977777826949f,
			0.407240209881218f, 0.41254261144808f, 0.417885068796976f, 0.423267667919539f, 0.428690494531971f,
			0.434153634077377f, 0.439657171728079f, 0.445201192387887f, 0.450785780694349f, 0.456411021020965f,
			0.462076997479369f, 0.467783793921492f, 0.473531493941681f, 0.479320180878805f, 0.485149937818323f,
			0.491020847594331f, 0.496932992791578f, 0.502886455747457f, 0.50888131855397f, 0.514917663059676f,
			0.520995570871595f, 0.527115123357109f, 0.533276401645826f, 0.539479486631421f, 0.545724458973463f,
			0.552011399099209f, 0.558340387205378f, 0.56471150325991f, 0.571124827003694f, 0.577580437952282f,
			0.584078415397575f, 0.590618838409497f, 0.597201785837643f, 0.603827336312907f, 0.610495568249093f,
			0.617206559844509f, 0.623960389083534f, 0.630757133738175f, 0.637596871369601f, 0.644479679329661f,
			0.651405634762384f, 0.658374814605461f, 0.665387295591707f, 0.672443154250516f, 0.679542466909286f,
			0.686685309694841f, 0.693871758534824f, 0.701101889159085f, 0.708375777101046f, 0.71569349769906f,
			0.723055126097739f, 0.730460737249286f, 0.737910405914797f, 0.745404206665559f, 0.752942213884326f,
			0.760524501766589f, 0.768151144321824f, 0.775822215374732f, 0.783537788566466f, 0.791297937355839f,
			0.799102735020525f, 0.806952254658248f, 0.81484656918795f, 0.822785751350956f, 0.830769873712124f,
			0.838799008660978f, 0.846873228412837f, 0.854992605009927f, 0.863157210322481f, 0.871367116049835f,
			0.879622393721502f, 0.887923114698241f, 0.896269350173118f, 0.904661171172551f, 0.913098648557343f,
			0.921581853023715f, 0.930110855104312f, 0.938685725169219f, 0.947306533426946f, 0.955973349925421f,
			0.964686244552961f, 0.973445287039244f, 0.982250546956257f, 0.991102093719252f, 1.0f
		};
	}

	// Public

	/// <summary>
	/// Defines the log level for an output log message
	/// </summary>
	public enum LogLevel : int {
		/// <summary>
		/// Logs are printed to console and log files
		/// </summary>
		Display,
		/// <summary>
		/// Logs are printed to console and log files with the yellow color
		/// </summary>
		Warning,
		/// <summary>
		/// Logs are printed to console and log files with the red color
		/// </summary>
		Error,
		/// <summary>
		/// Logs are printed to console and log files then crashes the application even if logging is disabled
		/// </summary>
		Fatal
	}



	/// <summary>
	/// Defines how to blend when changing view targets
	/// </summary>
	public enum BlendType : int {
		/// <summary>
		/// A simple linear interpolation
		/// </summary>
		Linear,
		/// <summary>
		/// A slight ease in and ease out, but amount of ease cannot be tweaked
		/// </summary>
		Cubic,
		/// <summary>
		/// Immediately accelerates, but smoothly decelerates into the target, ease amount can be controlled
		/// </summary>
		EaseIn,
		/// <summary>
		/// Smoothly accelerates, but does not decelerate into the target, ease amount can be controlled
		/// </summary>
		EaseOut,
		/// <summary>
		/// Smoothly accelerates and decelerates, ease amount can be controlled
		/// </summary>
		EaseInOut,
		/// <summary>
		/// The game's camera system has already performed the blending, the engine shouldn't blend at all
		/// </summary>
		PreBlended
	}


	/// <summary>
	/// Defines the pixel format
	/// </summary>
	public enum PixelFormat : int {
		/// <summary/>
		Unknown = 0,
		/// <summary/>
		A32B32G32R32F = 1,
		/// <summary/>
		B8G8R8A8 = 2,
		/// <summary/>
		G8 = 3,
		/// <summary/>
		G16 = 4,
		/// <summary/>
		DXT1 = 5,
		/// <summary/>
		DXT3 = 6,
		/// <summary/>
		DXT5 = 7,
		/// <summary/>
		UYVY = 8,
		/// <summary/>
		FloatRGB = 9,
		/// <summary/>
		FloatRGBA = 10,
		/// <summary/>
		DepthStencil = 11,
		/// <summary/>
		ShadowDepth = 12,
		/// <summary/>
		R32Float = 13,
		/// <summary/>
		G16R16 = 14,
		/// <summary/>
		G16R16F = 15,
		/// <summary/>
		G16R16FFilter = 16,
		/// <summary/>
		G32R32F = 17,
		/// <summary/>
		A2B10G10R10 = 18,
		/// <summary/>
		A16B16G16R16 = 19,
		/// <summary/>
		D24 = 20,
		/// <summary/>
		R16F = 21,
		/// <summary/>
		R16FFilter = 22,
		/// <summary/>
		BC5 = 23,
		/// <summary/>
		V8U8 = 24,
		/// <summary/>
		A1 = 25,
		/// <summary/>
		FloatR11G11B10 = 26,
		/// <summary/>
		A8 = 27,
		/// <summary/>
		R32UInt = 28,
		/// <summary/>
		R32SInt = 29,
		/// <summary/>
		PVRTC2 = 30,
		/// <summary/>
		PVRTC4 = 31,
		/// <summary/>
		R16UInt = 32,
		/// <summary/>
		R16SInt = 33,
		/// <summary/>
		R16G16B16A16UInt = 34,
		/// <summary/>
		R16G16B16A16SInt = 35,
		/// <summary/>
		R5G6B5UNorm = 36,
		/// <summary/>
		R8G8B8A8 = 37,
		/// <summary/>
		A8R8G8B8 = 38,
		/// <summary/>
		BC4 = 39,
		/// <summary/>
		R8G8 = 40,
		/// <summary/>
		ATCRGB = 41,
		/// <summary/>
		ATCRGBAE = 42,
		/// <summary/>
		ATCRGBAI = 43,
		/// <summary/>
		X24G8 = 44,
		/// <summary/>
		ETC1 = 45,
		/// <summary/>
		ETC2RGB = 46,
		/// <summary/>
		ETC2RGBA = 47,
		/// <summary/>
		R32G32B32A32UInt = 48,
		/// <summary/>
		R16G16UInt = 49,
		/// <summary/>
		ASTC4x4 = 50,
		/// <summary/>
		ASTC6x6 = 51,
		/// <summary/>
		ASTC8x8 = 52,
		/// <summary/>
		ASTC10x10 = 53,
		/// <summary/>
		ASTC12x12 = 54,
		/// <summary/>
		BC6H = 55,
		/// <summary/>
		BC7 = 56,
		/// <summary/>
		R8UInt = 57,
		/// <summary/>
		L8 = 58,
		/// <summary/>
		XGXR8 = 59,
		/// <summary/>
		R8G8B8A8UInt = 60,
		/// <summary/>
		R8G8B8A8SNorm = 61,
		/// <summary/>
		R16G16B16A16UNorm = 62,
		/// <summary/>
		R16G16B16A16SNorm = 63,
		/// <summary/>
		PLATFORMHDR0 = 64,
		/// <summary/>
		PLATFORMHDR1 = 65,
		/// <summary/>
		PLATFORMHDR2 = 66,
		/// <summary/>
		NV12 = 67,
		/// <summary/>
		R32G32UInt = 68,
		/// <summary/>
		ETC2R11EAC = 69,
		/// <summary/>
		ETC2RG11EAC = 70,
		/// <summary/>
		R8 = 71
	}

	

	/// <summary>
	/// A linear 32-bit floating-point RGBA color
	/// </summary>
	public partial struct LinearColor : IEquatable<LinearColor> {
		/// <summary>
		/// Initializes a new instance the linear color
		/// </summary>
		public LinearColor(float red, float green, float blue, float alpha = 1.0f) {
			r = red;
			g = green;
			b = blue;
			a = alpha;
		}

		/// <summary>
		/// Initializes a new instance the linear color
		/// </summary>
		public LinearColor(Vector3 value, float alpha = 1.0f) {
			r = value.X;
			g = value.Y;
			b = value.Z;
			a = alpha;
		}

		/// <summary>
		/// Initializes a new instance the linear color
		/// </summary>
		public LinearColor(Vector4 value) {
			r = value.X;
			g = value.Y;
			b = value.Z;
			a = value.W;
		}

		/// <summary>
		/// Initializes a new instance the linear color
		/// </summary>
		public LinearColor(Color value) {
			r = Tables.Color[value.R];
			g = Tables.Color[value.G];
			b = Tables.Color[value.B];
			a = Tables.Color[value.A];
		}

		/// <summary>
		/// Initializes a new instance the linear color
		/// </summary>
		public LinearColor(float[] values) {
			if (values == null)
				throw new ArgumentNullException(nameof(values));

			if (values.Length != 3 && values.Length != 4)
				throw new ArgumentOutOfRangeException(nameof(values));

			r = values[0];
			g = values[1];
			b = values[2];
			a = values.Length >= 4 ? values[3] : 1.0f;
		}

		/// <summary>
		/// Gets or sets the component at the specified index
		/// </summary>
		public float this[int index] {
			get {
				switch (index) {
					case 0: return r;
					case 1: return g;
					case 2: return b;
					case 3: return a;
				}

				throw new ArgumentOutOfRangeException(nameof(index));
			}

			set {
				switch (index) {
					case 0: r = value; break;
					case 1: g = value; break;
					case 2: b = value; break;
					case 3: a = value; break;
					default: throw new ArgumentOutOfRangeException(nameof(index));
				}
			}
		}

		/// <summary>
		/// Gets or sets the red component of the linear color
		/// </summary>
		public float R {
			get => r;
			set => r = value;
		}

		/// <summary>
		/// Gets or sets the green component of the linear color
		/// </summary>
		public float G {
			get => g;
			set => g = value;
		}

		/// <summary>
		/// Gets or sets the blue component of the linear color
		/// </summary>
		public float B {
			get => b;
			set => b = value;
		}

		/// <summary>
		/// Gets or sets the alpha component of the linear color
		/// </summary>
		public float A {
			get => a;
			set => a = value;
		}

		/// <summary>
		/// The black color
		/// </summary>
		public static LinearColor Black => new(0.0f, 0.0f, 0.0f, 1.0f);

		/// <summary>
		/// The blue color
		/// </summary>
		public static LinearColor Blue => new(0.0f, 0.0f, 1.0f, 1.0f);

		/// <summary>
		/// The green color
		/// </summary>
		public static LinearColor Green => new(0.0f, 1.0f, 0.0f, 1.0f);

		/// <summary>
		/// The grey color
		/// </summary>
		public static LinearColor Grey => new(0.5f, 0.5f, 0.5f, 1.0f);

		/// <summary>
		/// The red color
		/// </summary>
		public static LinearColor Red => new(1.0f, 0.0f, 0.0f, 1.0f);

		/// <summary>
		/// The white color
		/// </summary>
		public static LinearColor White => new(1.0f, 1.0f, 1.0f, 1.0f);

		/// <summary>
		/// The yellow color
		/// </summary>
		public static LinearColor Yellow => new(1.0f, 1.0f, 0.0f, 1.0f);

		/// <summary>
		/// Tests for equality between two objects
		/// </summary>
		public static bool operator ==(LinearColor left, LinearColor right) => left.Equals(right);

		/// <summary>
		/// Tests for inequality between two objects
		/// </summary>
		public static bool operator !=(LinearColor left, LinearColor right) => !left.Equals(right);

		/// <summary>
		/// Adds two colors
		/// </summary>
		public static LinearColor operator +(LinearColor left, LinearColor right) => new(left.r + right.r, left.g + right.g, left.b + right.b, left.a + right.a);

		/// <summary>
		/// Subtracts two colors
		/// </summary>
		public static LinearColor operator -(LinearColor left, LinearColor right) => new(left.b - right.b, left.g - right.g, left.b - right.b, left.a - right.a);

		/// <summary>
		/// Multiplies two colors
		/// </summary>
		public static LinearColor operator *(float scale, LinearColor value) => new(value.r * scale, value.g * scale, value.b * scale, value.a * scale);

		/// <summary>
		/// Divides two colors
		/// </summary>
		public static LinearColor operator /(float scale, LinearColor value) => new(value.r / scale, value.g / scale, value.b / scale, value.a / scale);

		/// <summary>
		/// Implicitly casts color instance to a linear color
		/// </summary>
		public static implicit operator LinearColor(Color value) => new(value);

		/// <summary>
		/// Implicitly casts this instance to a string
		/// </summary>
		public static implicit operator string(LinearColor value) => value.ToString();

		/// <summary>
		/// Adds two colors
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static LinearColor Add(LinearColor left, LinearColor right) => new(left.r + right.r, left.g + right.g, left.b + right.b, left.a + right.a);

		/// <summary>
		/// Subtracts two colors
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static LinearColor Subtract(LinearColor left, LinearColor right) => new(left.r - right.r, left.g - right.g, left.b - right.b, left.a - right.a);

		/// <summary>
		/// Multiplies two colors
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static LinearColor Multiply(LinearColor left, LinearColor right) => new(left.r * right.r, left.g * right.g, left.b * right.b, left.a * right.a);

		/// <summary>
		/// Divides two colors
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static LinearColor Divide(LinearColor left, LinearColor right) => new(left.r / right.r, left.g / right.g, left.b / right.b, left.a / right.a);

		/// <summary>
		/// Performs a linear interpolation between two colors
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static LinearColor Lerp(LinearColor start, LinearColor end, float amount) => new(Maths.Lerp(start.r, end.r, amount), Maths.Lerp(start.g, end.g, amount), Maths.Lerp(start.b, end.b, amount), Maths.Lerp(start.a, end.a, amount));

		/// <summary>
		/// Converts the color into a linear color
		/// </summary>
		public static LinearColor FromColor(Color value) => new(value);

		/// <summary>
		/// Converts the linear color into a three component vector
		/// </summary>
		public Vector3 ToVector3() => new(r, g, b);

		/// <summary>
		/// Converts the linear color into a four component vector
		/// </summary>
		public Vector4 ToVector4() => new(r, g, b, a);

		/// <summary>
		/// Creates an array containing the elements of the linear color
		/// </summary>
		public float[] ToArray() => new[] { r, g, b, a };

		/// <summary>
		/// Returns a string that represents this instance
		/// </summary>
		public override string ToString() => ToString(CultureInfo.CurrentCulture);

		/// <summary>
		/// Returns a string that represents this instance
		/// </summary>
		public string ToString(IFormatProvider formatProvider) => string.Format(formatProvider, "R:{0} G:{1} B:{2} A:{3}", R, G, B, A);

		/// <summary>
		/// Indicates equality of objects
		/// </summary>
		public bool Equals(LinearColor other) => r == other.r && g == other.g && b == other.b && a == other.a;

		/// <summary>
		/// Indicates equality of objects
		/// </summary>
		public override bool Equals(object value) {
			if (value == null)
				return false;

			if (!ReferenceEquals(value.GetType(), typeof(LinearColor)))
				return false;

			return Equals((LinearColor)value);
		}

		/// <summary>
		/// Returns a hash code for the object
		/// </summary>
		public override int GetHashCode() => HashCode.Combine(r, g, b, a);
	}

	
    /// <summary>
    /// A representation of the engine's object reference
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ObjectReference : IEquatable<ObjectReference>
    {
        private IntPtr pointer;

        internal IntPtr Pointer
        {
            get
            {


                return pointer;
            }

            set
            {


                pointer = value;
            }
        }

        /// <summary>
        /// Tests for equality between two objects
        /// </summary>
        public static bool operator ==(ObjectReference left, ObjectReference right) => left.Equals(right);

        /// <summary>
        /// Tests for inequality between two objects
        /// </summary>
        public static bool operator !=(ObjectReference left, ObjectReference right) => !left.Equals(right);

       

        /// <summary>
        /// Returns the name of the object
        /// </summary>
        public string Name
        {
            get
            {
                byte[] stringBuffer = ArrayPool.GetStringBuffer();

                Object.getName(Pointer, stringBuffer);

                return stringBuffer.BytesToString();
            }
        }

        /// <summary>
        /// Indicates equality of objects
        /// </summary>
        public bool Equals(ObjectReference other) => pointer == other.pointer;

        /// <summary>
        /// Indicates equality of objects
        /// </summary>
        public override bool Equals(object value)
        {
            if (value == null)
                return false;

            if (!ReferenceEquals(value.GetType(), typeof(ObjectReference)))
                return false;

            return Equals((ObjectReference)value);
        }

        /// <summary>
        /// Returns a hash code for the object
        /// </summary>
        public override int GetHashCode() => pointer.GetHashCode();

      
    }

    /// <summary>
    /// Provides additional static constants and methods for mathematical functions that are lack in <see cref="System.Math"/>, <see cref="System.MathF"/>, and <see cref="System.Numerics"/>
    /// </summary>
    public static class Maths {
		/// <summary>
		/// Degrees-to-radians conversion constant
		/// </summary>
		public const float DegToRadF = MathF.PI * 2.0f / 360.0f;

		/// <summary>
		/// Radians-to-degrees conversion constant
		/// </summary>
		public const float RadToDegF = 1.0f / DegToRadF;

		// Double-precision

		/// <summary>
		/// Returns the dot product of two float values
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Dot(double left, double right) => left * right;

		/// <summary>
		/// Clamps value between 0.0d and 1.0d
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Saturate(double value) => value < 0.0d ? 0.0d : value > 1.0d ? 1.0d : value;

		/// <summary>
		/// Returns the fractional part of a float value
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Fraction(double value) => value - Math.Floor(value);

		/// <summary>
		/// Calculates the shortest difference between the two given angles given in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double DeltaAngle(double current, double target) {
			double delta = Repeat((target - current), 360.0d);

			if (delta > 180.0d)
				delta -= 360.0d;

			return delta;
		}

		/// <summary>
		/// Returns the next power of two
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double NextPowerOfTwo(double value) => Math.Pow(2.0d, Math.Ceiling(Math.Log(value, 2.0d)));

		/// <summary>
		/// Returns the previous power of two
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double PreviousPowerOfTwo(double value) => Math.Pow(2.0d, Math.Floor(Math.Log(value, 2.0d)));

		/// <summary>
		/// Performs smooth (Cubic Hermite) interpolation between 0.0d and 1.0d
		/// </summary>
		/// <param name="amount">Value between 0.0d and 1.0d indicating interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double SmoothStep(double amount) => (amount <= 0.0d) ? 0.0d : (amount >= 1.0d) ? 1.0d : amount * amount * (3.0d - (2.0d * amount));

		/// <summary>
		/// Performs a smoother interpolation between 0.0d and 1.0d with 1st and 2nd order derivatives of zero at endpoints
		/// </summary>
		/// <param name="amount">>Value between 0.0d and 1.0d indicating interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double SmootherStep(double amount) => (amount <= 0.0d) ? 0.0d : (amount >= 1.0d) ? 1.0d : amount * amount * amount * (amount * ((amount * 6.0d) - 15.0d) + 10.0d);

		/// <summary>
		/// Loops the value so that it is never larger than length and never smaller than 0.0d
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Repeat(double value, double length) => Math.Clamp(value - Math.Floor(value / length) * length, 0.0d, length);

		/// <summary>
		/// Interpolates between two values linearly
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Lerp(double from, double to, double amount) => from + amount * (to - from);

		/// <summary>
		/// Interpolates between two values linearly, but makes sure the values calculated correctly when they wrap around 360 degrees
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double LerpAngle(double from, double to, double amount) {
			double delta = Repeat((to - from), 360.0d);

			if (delta > 180.0d)
				delta -= 360.0d;

			return from + delta * Saturate(amount);
		}

		/// <summary>
		/// Inverse-interpolates between two values linearly
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double InverseLerp(double from, double to, double amount) => from != to ? Saturate((amount - from) / (to - from)) : 0.0d;

		/// <summary>
		/// Creates framerate-independent dampened motion between two values
		/// </summary>
		/// <param name="from">Value to damp from</param>
		/// <param name="to">Value to damp to</param>
		/// <param name="lambda">Smoothing factor</param>
		/// <param name="deltaTime">Time since last damp</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double Damp(double from, double to, double lambda, double deltaTime) => Lerp(from, to, 1.0d - Math.Exp(-lambda * deltaTime));

		/// <summary>
		/// Returns the vector moved towards a target
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double MoveTowards(double current, double target, double maxDelta) => Math.Abs(target - current) <= maxDelta ? target : current + Math.Sign(target - current) * maxDelta;

		/// <summary>
		/// Returns the vector moved towards a target, but makes sure the values calculated correctly when they wrap around 360 degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static double MoveTowardsAngle(double current, double target, double maxDelta) {
			double deltaAngle = DeltaAngle(current, target);

			if (-maxDelta < deltaAngle && deltaAngle < maxDelta)
				return target;

			target = current + deltaAngle;

			return MoveTowards(current, target, maxDelta);
		}

		// Single-precision

		/// <summary>
		/// Returns the dot product of two float values
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(float left, float right) => left * right;

		/// <summary>
		/// Clamps value between 0.0f and 1.0f
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Saturate(float value) => value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;

		/// <summary>
		/// Returns the fractional part of a float value
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Fraction(float value) => value - MathF.Floor(value);

		/// <summary>
		/// Calculates the shortest difference between the two given angles given in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DeltaAngle(float current, float target) {
			float delta = Repeat((target - current), 360.0f);

			if (delta > 180.0f)
				delta -= 360.0f;

			return delta;
		}

		/// <summary>
		/// Returns the next power of two
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float NextPowerOfTwo(float value) => MathF.Pow(2.0f, MathF.Ceiling(MathF.Log(value, 2.0f)));

		/// <summary>
		/// Returns the previous power of two
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float PreviousPowerOfTwo(float value) => MathF.Pow(2.0f, MathF.Floor(MathF.Log(value, 2.0f)));

		/// <summary>
		/// Performs smooth (Cubic Hermite) interpolation between 0.0f and 1.0f
		/// </summary>
		/// <param name="amount">Value between 0.0f and 1.0f indicating interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SmoothStep(float amount) => (amount <= 0.0f) ? 0.0f : (amount >= 1.0f) ? 1.0f : amount * amount * (3.0f - (2.0f * amount));

		/// <summary>
		/// Performs a smoother interpolation between 0.0f and 1.0f with 1st and 2nd order derivatives of zero at endpoints
		/// </summary>
		/// <param name="amount">>Value between 0.0f and 1.0f indicating interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SmootherStep(float amount) => (amount <= 0.0f) ? 0.0f : (amount >= 1.0f) ? 1.0f : amount * amount * amount * (amount * ((amount * 6.0f) - 15.0f) + 10.0f);

		/// <summary>
		/// Loops the value so that it is never larger than length and never smaller than 0.0f
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Repeat(float value, float length) => Math.Clamp(value - MathF.Floor(value / length) * length, 0.0f, length);

		/// <summary>
		/// Interpolates between two values linearly
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Lerp(float from, float to, float amount) => from + amount * (to - from);

		/// <summary>
		/// Interpolates between two values linearly, but makes sure the values calculated correctly when they wrap around 360 degrees
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float LerpAngle(float from, float to, float amount) {
			float delta = Repeat((to - from), 360.0f);

			if (delta > 180.0f)
				delta -= 360.0f;

			return from + delta * Saturate(amount);
		}

		/// <summary>
		/// Inverse-interpolates between two values linearly
		/// </summary>
		/// <param name="from">Value to interpolate from</param>
		/// <param name="to">Value to interpolate to</param>
		/// <param name="amount">Interpolation amount</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float InverseLerp(float from, float to, float amount) => from != to ? Saturate((amount - from) / (to - from)) : 0.0f;

		/// <summary>
		/// Creates framerate-independent dampened motion between two values
		/// </summary>
		/// <param name="from">Value to damp from</param>
		/// <param name="to">Value to damp to</param>
		/// <param name="lambda">Smoothing factor</param>
		/// <param name="deltaTime">Time since last damp</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Damp(float from, float to, float lambda, float deltaTime) => Lerp(from, to, 1.0f - MathF.Exp(-lambda * deltaTime));

		/// <summary>
		/// Returns the vector moved towards a target
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float MoveTowards(float current, float target, float maxDelta) => MathF.Abs(target - current) <= maxDelta ? target : current + MathF.Sign(target - current) * maxDelta;

		/// <summary>
		/// Returns the vector moved towards a target, but makes sure the values calculated correctly when they wrap around 360 degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float MoveTowardsAngle(float current, float target, float maxDelta) {
			float deltaAngle = DeltaAngle(current, target);

			if (-maxDelta < deltaAngle && deltaAngle < maxDelta)
				return target;

			target = current + deltaAngle;

			return MoveTowards(current, target, maxDelta);
		}

		// Vector2

		/// <summary>
		/// Returns the length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Magnitude(Vector2 value) => MathF.Sqrt(SquareMagnitude(value));

		/// <summary>
		/// Returns the squared length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SquareMagnitude(Vector2 value) => Vector2.Dot(value, value);

		/// <summary>
		/// Returns the refraction vector
		/// </summary>
		/// <param name="value">The incident vector</param>
		/// <param name="normal">The normal vector</param>
		/// <param name="eta">The refraction index</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 Refract(Vector2 value, Vector2 normal, float eta) {
			float ni = Vector2.Dot(normal, value);
			float k = 1.0f - eta * eta * (1.0f - ni * ni);

			return k >= 0.0f ? eta * value - (eta * ni + MathF.Sqrt(k)) * normal : Vector2.Zero;
		}

		/// <summary>
		/// Returns the vector perpendicular to the specified vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 Perpendicular(Vector2 value) => new(-value.Y, value.X);

		/// <summary>
		/// Returns the unsigned angle in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Angle(Vector2 left, Vector2 right) {
			Vector2 lr = left * Magnitude(right);
			Vector2 rl = right * Magnitude(left);

			return 2.0f * MathF.Atan2(Magnitude(lr - rl), Magnitude(lr + rl)) * RadToDegF;
		}

		/// <summary>
		/// Returns a copy of vector with clamped magnitude
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 ClampMagnitude(Vector2 value, float maxLength) {
			float squareMagnitude = SquareMagnitude(value);

			if (squareMagnitude > maxLength * maxLength) {
				float magnitude = MathF.Sqrt(squareMagnitude);

				return new((value.X / magnitude) * maxLength, (value.Y / magnitude) * maxLength);
			}

			return value;
		}

		/// <summary>
		/// Creates framerate-independent dampened motion between two values
		/// </summary>
		/// <param name="from">Value to damp from</param>
		/// <param name="to">Value to damp to</param>
		/// <param name="lambda">Smoothing factor</param>
		/// <param name="deltaTime">Time since last damp</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 Damp(Vector2 from, Vector2 to, float lambda, float deltaTime) => Vector2.Lerp(from, to, 1.0f - MathF.Exp(-lambda * deltaTime));

		/// <summary>
		/// Returns the vector moved towards a target
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector2 MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta) {
			Vector2 destination = Vector2.Subtract(target, current);

			float squareMagnitude = SquareMagnitude(destination);

			if (squareMagnitude == 0.0f || (maxDistanceDelta >= 0.0f && squareMagnitude <= maxDistanceDelta * maxDistanceDelta))
				return target;

			float distance = MathF.Sqrt(squareMagnitude);

			return new(current.X + destination.X / distance * maxDistanceDelta, current.Y + destination.Y / distance * maxDistanceDelta);
		}

		// Vector3

		/// <summary>
		/// Returns the length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Magnitude(Vector3 value) => MathF.Sqrt(SquareMagnitude(value));

		/// <summary>
		/// Returns the squared length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SquareMagnitude(Vector3 value) => Vector3.Dot(value, value);

		/// <summary>
		/// Returns the refraction vector
		/// </summary>
		/// <param name="value">The incident vector</param>
		/// <param name="normal">The normal vector</param>
		/// <param name="eta">The refraction index</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Refract(Vector3 value, Vector3 normal, float eta) {
			float ni = Vector3.Dot(normal, value);
			float k = 1.0f - eta * eta * (1.0f - ni * ni);

			return k >= 0.0f ? eta * value - (eta * ni + MathF.Sqrt(k)) * normal : Vector3.Zero;
		}

		/// <summary>
		/// Returns the unsigned angle in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Angle(Vector3 left, Vector3 right) {
			Vector3 lr = left * Magnitude(right);
			Vector3 rl = right * Magnitude(left);

			return 2.0f * MathF.Atan2(Magnitude(lr - rl), Magnitude(lr + rl)) * RadToDegF;
		}

		/// <summary>
		/// Returns the signed angle in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SignedAngle(Vector3 left, Vector3 right, Vector3 axis) {
			Vector3 cross = Vector3.Cross(left, right);

			return Angle(left, right) * MathF.Sign(Vector3.Dot(axis, cross));
		}

		/// <summary>
		/// Returns a copy of vector with clamped magnitude
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 ClampMagnitude(Vector3 value, float maxLength) {
			float squareMagnitude = SquareMagnitude(value);

			if (squareMagnitude > maxLength * maxLength) {
				float magnitude = MathF.Sqrt(squareMagnitude);

				return new((value.X / magnitude) * maxLength, (value.Y / magnitude) * maxLength, (value.Z / magnitude) * maxLength);
			}

			return value;
		}

		/// <summary>
		/// Creates framerate-independent dampened motion between two values
		/// </summary>
		/// <param name="from">Value to damp from</param>
		/// <param name="to">Value to damp to</param>
		/// <param name="lambda">Smoothing factor</param>
		/// <param name="deltaTime">Time since last damp</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Damp(Vector3 from, Vector3 to, float lambda, float deltaTime) => Vector3.Lerp(from, to, 1.0f - MathF.Exp(-lambda * deltaTime));

		/// <summary>
		/// Returns the vector moved towards a target
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta) {
			Vector3 destination = Vector3.Subtract(target, current);

			float squareMagnitude = SquareMagnitude(destination);

			if (squareMagnitude == 0.0f || (maxDistanceDelta >= 0.0f && squareMagnitude <= maxDistanceDelta * maxDistanceDelta))
				return target;

			float distance = MathF.Sqrt(squareMagnitude);

			return new(current.X + destination.X / distance * maxDistanceDelta, current.Y + destination.Y / distance * maxDistanceDelta, current.Z + destination.Z / distance * maxDistanceDelta);
		}

		/// <summary>
		/// Projects a vector onto another vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Project(Vector3 value, Vector3 normal) {
			float squareMagnitude = SquareMagnitude(normal);

			if (squareMagnitude < Single.Epsilon)
				return Vector3.Zero;

			float dot = Vector3.Dot(value, normal);

			return new(normal.X * dot / squareMagnitude, normal.Y * dot / squareMagnitude, normal.Z * dot / squareMagnitude);
		}

		/// <summary>
		/// Projects a vector onto a plane defined by a normal orthogonal to the plane
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 ProjectOnPlane(Vector3 value, Vector3 planeNormal) {
			float squareMagnitude = SquareMagnitude(planeNormal);

			if (squareMagnitude < Single.Epsilon)
				return value;

			float dot = Vector3.Dot(value, planeNormal);

			return new(value.X - planeNormal.X * dot / squareMagnitude, value.Y - planeNormal.Y * dot / squareMagnitude, value.Z - planeNormal.Z * dot / squareMagnitude);
		}

		// Vector4

		/// <summary>
		/// Returns the length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Magnitude(Vector4 value) => MathF.Sqrt(SquareMagnitude(value));

		/// <summary>
		/// Returns the squared length of the vector
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float SquareMagnitude(Vector4 value) => Vector4.Dot(value, value);

		/// <summary>
		/// Returns the refraction vector
		/// </summary>
		/// <param name="value">The incident vector</param>
		/// <param name="normal">The normal vector</param>
		/// <param name="eta">The refraction index</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector4 Refract(Vector4 value, Vector4 normal, float eta) {
			float ni = Vector4.Dot(normal, value);
			float k = 1.0f - eta * eta * (1.0f - ni * ni);

			return k >= 0.0f ? eta * value - (eta * ni + MathF.Sqrt(k)) * normal : Vector4.Zero;
		}

		// Quaternion

		/// <summary>
		/// Returns a rotation which rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion Euler(float x, float y, float z) {
			x *= DegToRadF;
			y *= DegToRadF;
			z *= DegToRadF;

			return CreateFromYawPitchRoll(z, -y, -x);
		}

		/// <summary>
		/// Returns a rotation which rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion Euler(Vector3 eulerAngles) => Euler(eulerAngles.X, eulerAngles.Y, eulerAngles.Z);

		/// <summary>
		/// Returns the unsigned angle in degrees
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Angle(Quaternion from, Quaternion to) {
			float dot = Quaternion.Dot(from, to);

			return dot > 1.0f - 0.000001f ? 0.0f : MathF.Acos(MathF.Min(MathF.Abs(dot), 1.0f)) * 2.0f * RadToDegF;
		}

		/// <summary>
		/// Returns a rotation which rotates angle degrees around axis
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion AngleAxis(float angle, Vector3 axis) {
			axis = axis * DegToRadF;
			axis = Vector3.Normalize(axis);

			float halfAngle = angle * DegToRadF * 0.5f;
			float sin = MathF.Sin(halfAngle);

			return new(axis.X * sin, axis.Y * sin, axis.Z * sin, MathF.Cos(halfAngle));
		}

		/// <summary>
		/// Returns a rotation which rotates from <paramref name="fromDirection"/> to <paramref name="toDirection"/>
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection) {
			float dot = Vector3.Dot(fromDirection, toDirection);
			float normal = MathF.Sqrt(SquareMagnitude(fromDirection) * SquareMagnitude(toDirection));
			float real = normal + dot;
			Vector3 final = default;

			if (real < Single.Epsilon * normal) {
				real = 0.0f;
				final = MathF.Abs(fromDirection.X) > MathF.Abs(fromDirection.Z) ? new(-fromDirection.Y, fromDirection.X, 0.0f) : new(0.0f, -fromDirection.Z, fromDirection.Y);
			} else {
				final = Vector3.Cross(fromDirection, toDirection);
			}

			return Quaternion.Normalize(new(final, real));
		}

		/// <summary>
		/// Creates framerate-independent dampened motion between two values
		/// </summary>
		/// <param name="from">Value to damp from</param>
		/// <param name="to">Value to damp to</param>
		/// <param name="lambda">Smoothing factor</param>
		/// <param name="deltaTime">Time since last damp</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion Damp(Quaternion from, Quaternion to, float lambda, float deltaTime) => Quaternion.Slerp(from, to, 1.0f - MathF.Exp(-lambda * deltaTime));

		/// <summary>
		/// Returns a rotation which rotated towards a target
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxDegreesDelta) {
			float angle = Angle(from, to);

			if (angle == 0.0f)
				return to;

			return Quaternion.Slerp(from, to, MathF.Min(1.0f, maxDegreesDelta / angle));
		}

		/// <summary>
		/// Returns a rotation from the given yaw, pitch, and roll, in radians
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) {
			float sr, cr, sp, cp, sy, cy;
			float halfRoll = roll * 0.5f;

			sr = MathF.Sin(halfRoll);
			cr = MathF.Cos(halfRoll);

			float halfPitch = pitch * 0.5f;

			sp = MathF.Sin(halfPitch);
			cp = MathF.Cos(halfPitch);

			float halfYaw = yaw * 0.5f;

			sy = MathF.Sin(halfYaw);
			cy = MathF.Cos(halfYaw);

			Quaternion result = default;

			result.X = cy * cp * sr - sy * sp * cr;
			result.Y = cy * sp * cr + sy * cp * sr;
			result.Z = sy * cp * cr - cy * sp * sr;
			result.W = cy * cp * cr + sy * sp * sr;

			return result;
		}
	}

	/// <summary>
	/// Functionality to detect and diagnose unexpected or invalid runtime conditions during development, emitted if the assembly is built with the <a href="https://docs.microsoft.com/en-us/dotnet/core/tools/dotnet-build#options">Debug</a> configuration or if <c>ASSERTIONS</c> symbol is defined, signals a breakpoint to an attached debugger
	/// </summary>
	public static unsafe partial class Assert {
		[ThreadStatic]
		private static StringBuilder stringBuffer = new(8192);

		private static void Message(string message, int callerLineNumber, string callerFilePath) {
			stringBuffer.Clear()
			.AppendFormat("Assertion is failed at line {0} in file \"{1}\"", callerLineNumber, callerFilePath);

			if (message != null)
				stringBuffer.AppendFormat(" with message: {0}", message);

			outputMessage(stringBuffer.ToString().StringToBytes());

			Debugger.Break();
		}

		/// <summary>
		/// Logs an assertion if condition is <c>true</c>, and prints it on the screen
		/// </summary>
		[Conditional("DEBUG"), Conditional("ASSERTIONS")]
		public static void IsFalse(bool condition, string message = null, [CallerLineNumber] int callerLineNumber = 0, [CallerFilePath] string callerFilePath = null) {
			if (condition)
				Message(message, callerLineNumber, callerFilePath);
		}

		/// <summary>
		/// Logs an assertion if condition is <c>false</c>, and prints it on the screen
		/// </summary>
		[Conditional("DEBUG"), Conditional("ASSERTIONS")]
		public static void IsTrue(bool condition, string message = null, [CallerLineNumber] int callerLineNumber = 0, [CallerFilePath] string callerFilePath = null) {
			if (!condition)
				Message(message, callerLineNumber, callerFilePath);
		}

		/// <summary>
		/// Logs an assertion if value is not `null`, and prints it on the screen
		/// </summary>
		[Conditional("DEBUG"), Conditional("ASSERTIONS")]
		public static void IsNull<T>(T value, string message = null, [CallerLineNumber] int callerLineNumber = 0, [CallerFilePath] string callerFilePath = null) where T : class {
			if (value != null)
				Message(message, callerLineNumber, callerFilePath);
		}

		/// <summary>
		/// Logs an assertion if value is `null`, and prints it on the screen
		/// </summary>
		[Conditional("DEBUG"), Conditional("ASSERTIONS")]
		public static void IsNotNull<T>(T value, string message = null, [CallerLineNumber] int callerLineNumber = 0, [CallerFilePath] string callerFilePath = null) where T : class {
			if (value == null)
				Message(message, callerLineNumber, callerFilePath);
		}
	}

	/// <summary>
	/// Functionality for debugging
	/// </summary>
	public static unsafe partial class Debug {
		[ThreadStatic]
		private static StringBuilder stringBuffer = new(8192);

		/// <summary>
		/// Logs a message in accordance to the specified level, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void Log(LogLevel level, string message) {
			if (message == null)
				throw new ArgumentNullException(nameof(message));

			log(level, message.StringToBytes());
		}

		/// <summary>
		/// Creates a log file with the name of assembly if required and writes an exception to it, prints it on the screen, printing on the screen is omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration, but log file will persist
		/// </summary>
		public static void Exception(Exception exception) {
			if (exception == null)
				throw new ArgumentNullException(nameof(exception));

			stringBuffer.Clear()
			.AppendFormat("Time: {0}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss tt"))
			.AppendLine().AppendFormat("Message: {0}", exception.Message)
			.AppendLine().AppendFormat("StackTrace: {0}", exception.StackTrace)
			.AppendLine().AppendFormat("Source: {0}", exception.Source)
			.AppendLine();

			Debug.exception(stringBuffer.ToString().StringToBytes());

			using (StreamWriter streamWriter = File.AppendText(Directory.GetCurrentDirectory() + "Saved/Logs/Exceptions-" + Assembly.GetCallingAssembly().GetName().Name + ".log")) {
				streamWriter.WriteLine(stringBuffer);
				streamWriter.Close();
			}
		}

		/// <summary>
		/// Prints a debug message on the screen assigned to the key identifier, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void AddOnScreenMessage(int key, float timeToDisplay, Color displayColor, string message) {
			if (message == null)
				throw new ArgumentNullException(nameof(message));

			addOnScreenMessage(key, timeToDisplay, displayColor.ToArgb(), message.StringToBytes());
		}

		/// <summary>
		/// Clears any existing debug messages, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void ClearOnScreenMessages() => clearOnScreenMessages();

		/// <summary>
		/// Draws a debug box, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawBox(in Vector3 center, in Vector3 extent, in Quaternion rotation, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawBox(center, extent, rotation, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug capsule, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawCapsule(in Vector3 center, float halfHeight, float radius, in Quaternion rotation, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawCapsule(center, halfHeight, radius, rotation, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug cone, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawCone(in Vector3 origin, in Vector3 direction, float length, float angleWidth, float angleHeight, int sides, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawCone(origin, direction, length, angleWidth, angleHeight, sides, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug cylinder, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawCylinder(in Vector3 start, in Vector3 end, float radius, int segments, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawCylinder(start, end, radius, segments, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug sphere, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawSphere(in Vector3 center, float radius, int segments, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawSphere(center, radius, segments, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug line, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawLine(in Vector3 start, in Vector3 end, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0, float thickness = 0.0f) => drawLine(start, end, color.ToArgb(), persistentLines, lifeTime, depthPriority, thickness);

		/// <summary>
		/// Draws a debug point, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void DrawPoint(in Vector3 location, float size, Color color, bool persistentLines = false, float lifeTime = -1.0f, byte depthPriority = 0) => drawPoint(location, size, color.ToArgb(), persistentLines, lifeTime, depthPriority);

		/// <summary>
		/// Flushes persistent debug lines, omitted in builds with the <a href="https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html#buildconfigurationdescriptions">Shipping</a> configuration
		/// </summary>
		public static void FlushPersistentLines() => flushPersistentLines();
	}
	
}
