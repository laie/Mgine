#pragma once

namespace Mgine
{
	typedef float VEC;
	struct DROPFILE_INFO;

	typedef void (*CallbackInit)();
	typedef void (*CallbackStart)();
	typedef void (*CallbackUninit)();
	typedef void (*CallbackUpdate)(double ElapsedTime);
	typedef void (*CallbackDraw)();
	typedef void (*CallbackStreamUpdate)();
	typedef void (*CallbackDropFile)(DROPFILE_INFO *DropFileInfo);

	enum KEY_RAW_DATA : BYTE
	{
		KEY_RAW_UP = 0,
		KEY_RAW_DOWN
	};

	enum KEYSTATUS : BYTE
	{
		KEYSTATUS_UP = 0,
		KEYSTATUS_HIT,
		KEYSTATUS_DOWN
	};

	enum MOUSE_BUTTON : BYTE
	{
		MOUSE_LEFT = 0,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		MOUSE_ADDITIONAL0,
		MOUSE_ADDITIONAL1,
		MOUSE_ADDITIONAL2,
		MOUSE_ADDITIONAL3,
		MOUSE_ADDITIONAL4
	};

	enum WINDOW_HITTEST : BYTE
	{
		HITTEST_UNSET = 0xff,
		HITTEST_CONTENT = 0,
		HITTEST_MOVE,
		HITTEST_RESIZE_LEFT,
		HITTEST_RESIZE_RIGHT,
		HITTEST_RESIZE_TOPLEFT,
		HITTEST_RESIZE_TOP,
		HITTEST_RESIZE_TOPRIGHT,
		HITTEST_RESIZE_BOTTOMLEFT,
		HITTEST_RESIZE_BOTTOM,
		HITTEST_RESIZE_BOTTOMRIGHT,
	};

	enum SOUND_FORMAT : BYTE
	{
		SOUND_FORMAT_UNKNOWN = 0,
		SOUND_FORMAT_WAV,
		SOUND_FORMAT_MP3,
		SOUND_FORMAT_OGG
	};

	enum LOCALSETTING_TYPE : BYTE
	{
		LOCALSETTING_BINARY = 0,
		LOCALSETTING_DWORD,
		LOCALSETTING_QWORD,
		LOCALSETTING_STRING
	};

	enum TEXTUREADDRESS 
	{
		TEXTUREADDRESS_INHERIT		= -1,
		TEXTUREADDRESS_WRAP           = 0,
		TEXTUREADDRESS_MIRROR         = 1,
		TEXTUREADDRESS_CLAMP          = 2,
		TEXTUREADDRESS_BORDER         = 3,
		TEXTUREADDRESS_MIRRORONCE     = 4,
	};

	enum KEYCODE : BYTE
	{
		KeyEscape		=	0x01,
		Key1			=	0x02,
		Key2			=	0x03,
		Key3			=	0x04,
		Key4			=	0x05,
		Key5			=	0x06,
		Key6			=	0x07,
		Key7			=	0x08,
		Key8			=	0x09,
		Key9			=	0x0A,
		Key0			=	0x0B,
		KeyMinus		=	0x0C,    /* - on main keyboard */
		KeyEquals		=	0x0D,
		KeyBackspace	=	0x0E,    /* backspace */
		KeyTab			=	0x0F,
		KeyQ			=	0x10,
		KeyW			=	0x11,
		KeyE			=	0x12,
		KeyR			=	0x13,
		KeyT			=	0x14,
		KeyY			=	0x15,
		KeyU			=	0x16,
		KeyI			=	0x17,
		KeyO			=	0x18,
		KeyP			=	0x19,
		KeyLBracket		=	0x1A,
		KeyRBracket		=	0x1B,
		KeyReturn		=	0x1C,    /* Enter on main keyboard */
		KeyEnter		=	KeyReturn,
		KeyLControl		=	0x1D,
		KeyA			=	0x1E,
		KeyS			=	0x1F,
		KeyD			=	0x20,
		KeyF			=	0x21,
		KeyG			=	0x22,
		KeyH			=	0x23,
		KeyJ			=	0x24,
		KeyK			=	0x25,
		KeyL			=	0x26,
		KeySemicolon	=	0x27,
		KeyApostrophe	=	0x28,
		KeyGrave		=	0x29,    /* accent grave(`) */
		KeyLShift		=	0x2A,
		KeyBackslash	=	0x2B,
		KeyZ			=	0x2C,
		KeyX			=	0x2D,
		KeyC			=	0x2E,
		KeyV			=	0x2F,
		KeyB			=	0x30,
		KeyN			=	0x31,
		KeyM			=	0x32,
		KeyComma		=	0x33,
		KeyPeriod		=	0x34,    /* . on main keyboard */
		KeySlash		=	0x35,    /* / on main keyboard */
		KeyRShift		=	0x36,
		KeyMultiply		=	0x37,    /* * on numeric keypad */
		KeyLMenu		=	0x38,    /* left Alt */
		KeySpace		=	0x39,
		KeyCapital		=	0x3A,
		KeyF1			=	0x3B,
		KeyF2			=	0x3C,
		KeyF3			=	0x3D,
		KeyF4			=	0x3E,
		KeyF5			=	0x3F,
		KeyF6			=	0x40,
		KeyF7			=	0x41,
		KeyF8			=	0x42,
		KeyF9			=	0x43,
		KeyF10			=	0x44,
		KeyNumLock		=	0x45,
		KeyScroll		=	0x46,    /* Scroll Lock */
		KeyNumPad7		=	0x47,
		KeyNumPad8		=	0x48,
		KeyNumPad9		=	0x49,
		KeySubtract		=	0x4A,    /* - on numeric keypad */
		KeyNumPad4		=	0x4B,
		KeyNumPad5		=	0x4C,
		KeyNumPad6		=	0x4D,
		KeyAdd			=	0x4E,    /* + on numeric keypad */
		KeyNumPad1		=	0x4F,
		KeyNumPad2		=	0x50,
		KeyNumPad3		=	0x51,
		KeyNumPad0		=	0x52,
		KeyDecimal		=	0x53,    /* . on numeric keypad */
		KeyBackSlash	=	0x56,    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
		KeyF11			=	0x57,
		KeyF12			=	0x58,
		KeyF13			=	0x64,    /*                     (NEC PC98) */
		KeyF14			=	0x65,    /*                     (NEC PC98) */
		KeyF15			=	0x66,    /*                     (NEC PC98) */
		KeyKana			=	0x70,    /* (Japanese keyboard)            */
		KeyABNT_C1		=	0x73,    /* /? on Brazilian keyboard */
		KeyConvert		=	0x79,    /* (Japanese keyboard)            */
		KeyNoConvert	=	0x7B,    /* (Japanese keyboard)            */
		KeyYen			=	0x7D,    /* (Japanese keyboard)            */
		KeyABNT_C2		=	0x7E,    /* Numpad . on Brazilian keyboard */
		KeyNumPadEquals	=	0x8D,    /* = on numeric keypad (NEC PC98) */
		KeyPrevTrack	=	0x90,    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
		KeyAT			=	0x91,    /*                     (NEC PC98) */
		KeyColon		=	0x92,    /*                     (NEC PC98) */
		KeyUnderLine	=	0x93,    /*                     (NEC PC98) */
		KeyKanji		=	0x94,    /* (Japanese keyboard)            */
		KeyStop			=	0x95,    /*                     (NEC PC98) */
		KeyAX			=	0x96,    /*                     (Japan AX) */
		KeyUnLabeled	=	0x97,    /*                        (J3100) */
		KeyNextTrack	=	0x99,    /* Next Track */
		KeyNumPadEnter	=	0x9C,    /* Enter on numeric keypad */
		KeyRControl		=	0x9D,
		KeyMute			=	0xA0,    /* Mute */
		KeyCalculator	=	0xA1,    /* Calculator */
		KeyPlayPause	=	0xA2,    /* Play / Pause */
		KeyMediaStop	=	0xA4,    /* Media Stop */
		KeyVolumeDown	=	0xAE,    /* Volume - */
		KeyVolumeUp		=	0xB0,    /* Volume + */
		KeyWebHome		=	0xB2,    /* Web home */
		KeyNumPadComma	=	0xB3,    /* , on numeric keypad (NEC PC98) */
		KeyDivide		=	0xB5,    /* / on numeric keypad */
		KeySysRq		=	0xB7,
		KeyRMenu		=	0xB8,    /* right Alt */
		KeyPause		=	0xC5,    /* Pause */
		KeyHome			=	0xC7,    /* Home on arrow keypad */
		KeyUp			=	0xC8,    /* UpArrow on arrow keypad */
		KeyPrior		=	0xC9,    /* PgUp on arrow keypad */
		KeyLeft			=	0xCB,    /* LeftArrow on arrow keypad */
		KeyRight		=	0xCD,    /* RightArrow on arrow keypad */
		KeyEnd			=	0xCF,    /* End on arrow keypad */
		KeyDown			=	0xD0,    /* DownArrow on arrow keypad */
		KeyNext			=	0xD1,    /* PgDn on arrow keypad */
		KeyInsert		=	0xD2,    /* Insert on arrow keypad */
		KeyDelete		=	0xD3,    /* Delete on arrow keypad */
		KeyLWin			=	0xDB,    /* Left Windows key */
		KeyRWin			=	0xDC,    /* Right Windows key */
		KeyApps			=	0xDD,    /* AppMenu key */
		KeyPower		=	0xDE,    /* System Power */
		KeySleep		=	0xDF,    /* System Sleep */
		KeyWake			=	0xE3,    /* System Wake */
		KeyWebSearch	=	0xE5,    /* Web Search */
		KeyWebFavorites	=	0xE6,    /* Web Favorites */
		KeyWebRefresh	=	0xE7,    /* Web Refresh */
		KeyWebStop		=	0xE8,    /* Web Stop */
		KeyWebForward	=	0xE9,    /* Web Forward */
		KeyWebBack		=	0xEA,    /* Web Back */
		KeyMyComputer	=	0xEB,    /* My Computer */
		KeyMail			=	0xEC,    /* Mail */
		KeyMediaSelect	=	0xED,    /* Media Select */	

		/*
		 *  Alternate names for keys, to facilitate transition from DOS.
		 */
		KeyNumPadStar	=	KeyMultiply,        /* * on numeric keypad */
		KeyLalt			=	KeyLMenu,           /* left Alt */
		KeyCapsLock		=	KeyCapital,         /* CapsLock */
		KeyNumPadMinus	=	KeySubtract,        /* - on numeric keypad */
		KeyNumPadPlus	=	KeyAdd,             /* + on numeric keypad */
		KeyNumPadPeriod	=	KeyDecimal,         /* . on numeric keypad */
		KeyNumPadSlash	=	KeyDivide,          /* / on numeric keypad */
		KeyRalt			=	KeyRMenu,           /* right Alt */
		KeyUpArrow		=	KeyUp,              /* UpArrow on arrow keypad */
		KeyPgUp			=	KeyPrior,           /* PgUp on arrow keypad */
		KeyLeftArrow	=	KeyLeft,            /* LeftArrow on arrow keypad */
		KeyRightArrow	=	KeyRight,           /* RightArrow on arrow keypad */
		KeyDownArrow	=	KeyDown,            /* DownArrow on arrow keypad */
		KeyPgDn			=	KeyNext,            /* PgDn on arrow keypad */
	};

	enum CHARCODE : wchar_t
	{
		CharBackspace	= 0x0008,
		CharTab			= 0x0009,	// '	'
	
		CharCtrlA		= 0x0001,
		CharCtrlB		= 0x0002,
		CharCtrlC		= 0x0003,
		CharCtrlD		= 0x0004,
		CharCtrlE		= 0x0005,
		CharCtrlF		= 0x0006,
		CharCtrlG		= 0x0007,
		CharCtrlH		= 0x0008,
		CharCtrlI		= 0x0009,
		CharCtrlJ		= 0x000A,
		CharCtrlK		= 0x000B,
		CharCtrlL		= 0x000C,
		CharCtrlM		= 0x000D,
		CharCtrlN		= 0x000E,
		CharCtrlO		= 0x000F,
		CharCtrlP		= 0x0010,
		CharCtrlQ		= 0x0011,
		CharCtrlR		= 0x0012,
		CharCtrlS		= 0x0013,
		CharCtrlT		= 0x0014,
		CharCtrlU		= 0x0015,
		CharCtrlV		= 0x0016,
		CharCtrlW		= 0x0017,
		CharCtrlX		= 0x0018,
		CharCtrlY		= 0x0019,
		CharCtrlZ		= 0x001A,
		CharCtrlLBracket		= 0x001B,
		CharCtrlKeyBackSlash	= 0x001C,
		CharCtrlRBracket		= 0x001D,
	
		CharReturn		= 0x000D,
		CharEnter		= CharReturn,		// '\n'

		CharSpace			= 0x0020,		// ' '
		CharExclamationMark	= 0x0021,		// '!'
		CharQuotationMark	= 0x0022,		// '\"'
		CharSharp			= 0x0023,	// '#'
		CharDollar			= 0x0024,	// '$'
		CharPercent			= 0x0025,	// '%'
		CharAnd				= 0x0026,	// '&'
		CharApostrophe		= 0x0027,	// '\''
		CharLParenthes		= 0x0028,	// '('
		CharRParenthes		= 0x0029,	// ')'
		CharMultiply		= 0x002A,	// '*'
		CharPlus			= 0x002B,	// '+'
		CharComma			= 0x002C,	// ','
		CharMinus			= 0x002D,	// '-'
		CharPeriod			= 0x002E,	// '.'
		CharSlash			= 0x002F,	// '/'
		Char0				= 0x0030,	// '0'
		Char1				= 0x0031,	// '1'
		Char2				= 0x0032,	// '2'
		Char3				= 0x0033,	// '3'
		Char4				= 0x0034,	// '4'
		Char5				= 0x0035,	// '5'
		Char6				= 0x0036,	// '6'
		Char7				= 0x0037,	// '7'
		Char8				= 0x0038,	// '8'
		Char9				= 0x0039,	// '9'
		CharColon			= 0x003A,	// ':'
		CharSemicolon		= 0x003B,	// ';'
		CharLChevron		= 0x003C,	// '<'
		CharEqual			= 0x003D,	// '='
		CharRChevron		= 0x003E,	// '>'
		CharQuestionMark	= 0x003F,	// '?'
		CharAtSign			= 0x0040,	// '@'
		CharA				= 0x0041,	// 'A'
		CharB				= 0x0042,	// 'B'
		CharC				= 0x0043,	// 'C'
		CharD				= 0x0044,	// 'D'
		CharE				= 0x0045,	// 'E'
		CharF				= 0x0046,	// 'F'
		CharG				= 0x0047,	// 'G'
		CharH				= 0x0048,	// 'H'
		CharI				= 0x0049,	// 'I'
		CharJ				= 0x004A,	// 'J'
		CharK				= 0x004B,	// 'K'
		CharL				= 0x004C,	// 'L'
		CharM				= 0x004D,	// 'M'
		CharN				= 0x004E,	// 'N'
		CharO				= 0x004F,	// 'O'
		CharP				= 0x0050,	// 'P'
		CharQ				= 0x0051,	// 'Q'
		CharR				= 0x0052,	// 'R'
		CharS				= 0x0053,	// 'S'
		CharT				= 0x0054,	// 'T'
		CharU				= 0x0055,	// 'U'
		CharV				= 0x0056,	// 'V'
		CharW				= 0x0057,	// 'W'
		CharX				= 0x0058,	// 'X'
		CharY				= 0x0059,	// 'Y'
		CharZ				= 0x005A,	// 'Z'
		CharLBracket		= 0x005B,	// '['
		CharBackSlash		= 0x005C,	// '\'
		CharRBracket		= 0x005D,	// ']'
		CharCaret			= 0x005E,	// '^'
		CharUnderbar		= 0x005F,	// '_'
		CharGraveAccent		= 0x0060,	// '`'
		CharBackTick		= CharGraveAccent,
		Chara				= 0x0061,	// 'a'
		Charb				= 0x0062,	// 'b'
		Charc				= 0x0063,	// 'c'
		Chard				= 0x0064,	// 'd'
		Chare				= 0x0065,	// 'e'
		Charf				= 0x0066,	// 'f'
		Charg				= 0x0067,	// 'g'
		Charh				= 0x0068,	// 'h'
		Chari				= 0x0069,	// 'i'
		Charj				= 0x006A,	// 'j'
		Chark				= 0x006B,	// 'k'
		Charl				= 0x006C,	// 'l'
		Charm				= 0x006D,	// 'm'
		Charn				= 0x006E,	// 'n'
		Charo				= 0x006F,	// 'o'
		Charp				= 0x0070,	// 'p'
		Charq				= 0x0071,	// 'q'
		Charr				= 0x0072,	// 'r'
		Chars				= 0x0073,	// 's'
		Chart				= 0x0074,	// 't'
		Charu				= 0x0075,	// 'u'
		Charv				= 0x0076,	// 'v'
		Charw				= 0x0077,	// 'w'
		Charx				= 0x0078,	// 'x'
		Chary				= 0x0079,	// 'y'
		Charz				= 0x007A,	// 'z'
		CharLCurlyBracket	= 0x007B,	// '{'
		CharVerticalBar		= 0x007C,	// '|'
		CharRCurlyBracket	= 0x007D,	// '}'
		CharTilde			= 0x007E,	// '~'
		// Others are too many to include..
	};

	typedef struct MGINE_SOUND_DESC
	{
		void Uninit();
		
		SOUND_FORMAT		Format;
		BYTE				*Buffer;
		int					BufferSize;
		tWAVEFORMATEX		Wave;

		//int NumberOfChannels = 0, samplerate = 0, bytespersec = 0, blockalign = 0
	} *PMGINE_SOUND_DESC;

	enum EFFECT_VALUE_TYPE : BYTE
	{
		EFFECT_ARRAY = 0,
		EFFECT_BOOL,
		EFFECT_BOOL_ARRAY,
		EFFECT_INT,
		EFFECT_INT_ARRAY,
		EFFECT_FLOAT,
		EFFECT_FLOAT_ARRAY,
		EFFECT_TEXTURE
	};

	enum PRIM_TYPE : BYTE
	{
		PRIM_NONE = 0,
		PRIM_POINT,
		PRIM_LINE,
		PRIM_TRIPLE,
		PRIM_CUSTOMTRI
	};

	typedef struct DROPFILE_INFO
	{
		int FileCount;
		wchar_t (*FilePath)[MAX_PATH];
	} *PMGINE_DROPFILE_INFO;

	typedef struct MOUSE_STATE // equivalent with _DIMOUSESTATE2
	{ 
		LONG X;
		LONG Y;
		LONG Z;
		KEY_RAW_DATA Key[8];

		inline bool operator ==(const MOUSE_STATE & A) const
		{ return this->X == A.X && this->Y == A.Y && this->Z == A.Z && !memcmp(this->Key, A.Key, sizeof(this->Key)); }
		inline bool operator !=(const MOUSE_STATE & A) const
		{ return this->X != A.X || this->Y != A.Y || this->Z != A.Z || memcmp(this->Key, A.Key, sizeof(this->Key)); }
	} *PMGINE_MOUSE_STATE;

	typedef struct KEY_STATE
	{
		KEY_RAW_DATA Key[256];
		inline bool operator ==(const KEY_STATE & A) const
		{ return !memcmp(this, &A, sizeof(KEY_STATE)); }
		inline bool operator !=(const KEY_STATE & A) const
		{ return 0 != memcmp(this, &A, sizeof(KEY_STATE)); }
	} *PKEY_STATE;

	typedef struct EFFECT_CONSTANT
	{
		void Uninit();
		
		EFFECT_VALUE_TYPE Type;
		char Ident[128];
		DWORD Cb;
		void *Data;
	} *PMGINE_EFFECT_CONSTANT;

	typedef struct RECTANGLE
	{
		VEC X;
		VEC Y;
		VEC Width;
		VEC Height;

		inline RECTANGLE()
		{ X = 0; Y = 0; Width = 0; Height = 0; }

		inline RECTANGLE(VEC X, VEC Y, VEC Width, VEC Height)
		{
			this->X = X;
			this->Y = Y;
			this->Width = Width;
			this->Height = Height;
		}

		inline bool operator == (const RECTANGLE &A) const 
		{ return A.X == this->X && A.Y == this->Y && A.Width == this->Width && A.Height == this->Height; }
		inline bool operator != (const RECTANGLE &A) const
		{ return A.X != this->X || A.Y != this->Y || A.Width != this->Width || A.Height != this->Height; }
	
	} *PRECTANGLE;

#pragma pack(1)
	typedef struct COLOR
	{
		BYTE B, G, R, A;
		
		inline COLOR(){ B=G=R=A=0; }
		inline COLOR(DWORD Color){ *(DWORD*)&B= Color; }
		inline COLOR(BYTE A, BYTE R, BYTE G, BYTE B){ this->A=A; this->R=R; this->G=G; this->B=B; }
		inline COLOR(VEC A, VEC R, VEC G, VEC B){ this->A=(BYTE)(A*255); this->R=(BYTE)(R*255); this->G= (BYTE)(G*255); this->B=(BYTE)(B*255); }
		
		inline operator DWORD&() const { return *(DWORD*)&B; }

		inline COLOR & operator=(const COLOR & O)
		{ B=O.B; G=O.G; R=O.R; A=O.A; return *this; }

	} *PCOLOR;

	typedef struct VECTOR2
	{
		VEC X;
		VEC Y;

		inline VECTOR2& operator += ( const VECTOR2& A )
		{
			this->X += A.X;
			this->Y += A.Y;
			return *this;
		}

		inline VECTOR2& operator -= ( const VECTOR2& A )
		{
			this->X -= A.X;
			this->Y -= A.Y;
			return *this;
		}
		inline VECTOR2& operator *= ( const VEC& A )
		{
			this->X *= A;
			this->Y *= A;
			return *this;
		}
		inline VECTOR2& operator /= ( const VEC& A )
		{
			this->X /= A;
			this->Y /= A;
			return *this;
		}

		inline VECTOR2 operator + ( const VECTOR2& A ) const
		{
			VECTOR2 vec = *this;
			return vec += A;
		}
		inline VECTOR2 operator - ( const VECTOR2& A ) const
		{
			VECTOR2 vec = *this;
			return vec -= A;
		}

		inline bool operator == ( const VECTOR2& A ) const
		{ return A.X == this->X && A.Y == this->Y; }
		inline bool operator != ( const VECTOR2& A ) const
		{ return A.X != this->X || A.Y != this->Y; }

		inline operator const D3DXVECTOR2 & () const
		{ return *(D3DXVECTOR2*)this; }
	} *PMGINE_VECTOR2, MGINE_POINT, *PMGINE_VECTOR2;

	typedef struct VECTOR3
	{    
		VEC X, Y, Z;

		inline VECTOR3& operator += ( const VECTOR3& A )
		{
			this->X += A.X;
			this->Y += A.Y;
			this->Z += A.Z;
			return *this;
		}
		inline VECTOR3& operator -= ( const VECTOR3& A )
		{
			this->X -= A.X;
			this->Y -= A.Y;
			this->Z -= A.Z;
			return *this;
		}
		inline VECTOR3& operator *= ( const VEC& A )
		{
			this->X *= A;
			this->Y *= A;
			this->Z *= A;
			return *this;
		}
		inline VECTOR3& operator /= ( const VEC& A )
		{
			this->X /= A;
			this->Y /= A;
			this->Z /= A;
			return *this;
		}

		inline VECTOR3 operator + ( const VECTOR3& A ) const 
		{
			VECTOR3 vec = *this;
			return vec += A;
		}

		inline VECTOR3 operator - ( const VECTOR3& A ) const
		{
			VECTOR3 vec = *this;
			return vec -= A;
		}

		inline bool operator == ( const VECTOR3& A ) const
		{ return A.X == this->X && A.Y == this->Y && A.Z == this->Z; }

		inline bool operator != ( const VECTOR3& A ) const
		{ return A.X != this->X || A.Y != this->Y || A.Z != this->Z; }


	} *PMGINE_VECTOR3;

	typedef struct VERTEX_DOT
	{
		float X;
		float Y;
		float Z;
		//TVECTOR Rhw;
		COLOR ColorDiffuse;
		float U;
		float V;

		inline VERTEX_DOT()
		{ Default(); }

		void Default();
		void Transform(const D3DXMATRIXA16 & Matrix);
		void AdjustRadian(VEC Radian);
		void AdjustRadian(VEC Radian, VEC HotspotX, VEC HotspotY);
	} *PVERTEX_DOT;

	typedef struct VERTEX_LINE
	{
		VERTEX_DOT V1;
		VERTEX_DOT V2;

		void Transform(const D3DXMATRIXA16 & Matrix);
	} *PVERTEX_LINE;

	typedef struct VERTEX_TRIANGLE
	{
		VERTEX_DOT V1;
		VERTEX_DOT V2;
		VERTEX_DOT V3;

		void Transform(const D3DXMATRIXA16 & Matrix);
	} *PVERTEX_TRIANGLE;

	typedef struct VERTEX_TEXTURE
	{
		VERTEX_DOT V1;
		VERTEX_DOT V2;
		VERTEX_DOT V3;
		VERTEX_DOT V4;

		void Transform(const D3DXMATRIXA16 & Matrix);
	} *PVERTEX_TEXTURE;
#pragma pack()

	typedef struct LAYOUT_OFFSET
	{
		RECT Margin;
		RECT Padding;
		
		inline LAYOUT_OFFSET()
		{
			Margin.left = 0;
			Margin.top = 0;
			Margin.right = 0;
			Margin.bottom = 0;

			Padding.left = 0;
			Padding.top = 0;
			Padding.right = 0;
			Padding.bottom = 0;
		}

		inline LAYOUT_OFFSET(
			LONG MarginLeft,
			LONG MarginTop,
			LONG MarginRight,
			LONG MarginBottom,
			LONG PaddingLeft,
			LONG PaddingTop,
			LONG PaddingRight,
			LONG PaddingBottom)
		{
			Margin.left = MarginLeft;
			Margin.top = MarginTop;
			Margin.right = MarginRight;
			Margin.bottom = MarginBottom;

			Padding.left = PaddingLeft;
			Padding.top = PaddingTop;
			Padding.right = PaddingRight;
			Padding.bottom = PaddingBottom;
		}

		inline bool operator==( LAYOUT_OFFSET const & A ) const
		{ return 0 == memcmp(this, &A, sizeof(*this)); }
		inline bool operator!=( LAYOUT_OFFSET const & A ) const
		{ return 0 != memcmp(this, &A, sizeof(*this)); }
	} *PLAYOUT_OFFSET;

}