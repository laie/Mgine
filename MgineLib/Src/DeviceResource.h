#pragma once
#include "Engine.h"
#include "Event.h"

namespace Mgine
{
	class IndexBuffer;
	class TextureStatic;
	class Effect;
	class Font;

	class DeviceResourceManager
	{
	private:
		static Util::TriList<IndexBuffer*> *iListIndexBuffer;
		static Util::TriList<TextureStatic*> *iListTexture;
		static Util::TriList<Effect*> *iListEffect;
		static Util::TriList<Font*> *iListFont;


		static bool SubOnDeviceCreateIndexBuffer();

	public:
		static bool IsInited;

		static inline Util::TriList<IndexBuffer*> & ListIndexBuffer()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListIndexBuffer ) iListIndexBuffer = new Util::TriList<IndexBuffer*>;
			return *iListIndexBuffer;
		}
		static inline Util::TriList<TextureStatic*> & ListTexture()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListTexture ) iListTexture = new Util::TriList<TextureStatic*>;
			return *iListTexture;
		}
		static inline Util::TriList<Effect*> & ListEffect()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListEffect ) iListEffect = new Util::TriList<Effect*>;
			return *iListEffect;
		}
		static inline Util::TriList<Font*> & ListFont()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListFont ) iListFont = new Util::TriList<Font*>;
			return *iListFont;
		}

		static Effect DefaultVertexShader;
		static ID3DXSprite *SpriteFont;
		static Font DefaultFont;

		static void Init();
		static void Uninit();

		static void OnAppFinish();

		static void OnDeviceCreate();
		static void OnDeviceLost();
		static void OnDeviceReset();
	};

	interface IDeviceResource
	{
	public:
		enum TYPE_RESOURCE
		{
			RESOURCE_TYPE_UNSET = 0,
			INDEX_BUFFER,
			TEXTURE,
			RENDER_TARGET,
			EFFECT,
			FONT,
			CAMERA,
		};
	protected:
		PROPERTY_PROVIDE(IDeviceResource);
		DECLARE_PROP_TYPE_R(IDeviceResource, TYPE_RESOURCE, ResourceType, { return ResourceType.Value; }, { ResourceType.Value = Value; } );

		DECLARE_PROP_TYPE_R(IDeviceResource, bool, IsInited, { return IsInited.Value; }, { IsInited.Value = Value; IsDeviceLost = false; } );
		DECLARE_PROP_TYPE_R(IDeviceResource, bool, IsDeviceLost, { return IsDeviceLost.Value; }, { IsDeviceLost.Value = Value; } );


		inline IDeviceResource()
		{
			IsInited = false;
			IsDeviceLost = false;
			ResourceType = RESOURCE_TYPE_UNSET;
		}

	public:
		DECLARE_PROPERTY(ResourceType);
		DECLARE_PROPERTY(IsInited);
		DECLARE_PROPERTY(IsDeviceLost);


		virtual void Uninit()  = 0;

		virtual void RaiseDeviceLost();
		virtual void RaiseDeviceReset();
		virtual void RaiseDeviceCreate();
		
	protected:
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceReset() = 0;
		virtual void OnDeviceCreate() = 0;
		
	};



	class IndexBuffer :
		public IDeviceResource
	{
	private:
		IDirect3DIndexBuffer9 *gIndexBuffer;

	public:
		DWORD IndexBufferIndex;
		D3DINDEXBUFFER_DESC DescBuffer;

		inline IndexBuffer()
		{
			__SetResourceType(
					IDeviceResource::INDEX_BUFFER
				);
			gIndexBuffer = 0;
			memset(&DescBuffer, 0, sizeof(D3DINDEXBUFFER_DESC));
			IndexBufferIndex = DeviceResourceManager::ListIndexBuffer().Add(this);
		}
		virtual ~IndexBuffer();

		virtual void Init(WORD *Indices, DWORD CountIndices);
		virtual void Uninit();
		
		virtual IDirect3DIndexBuffer9 *GetIndexBuffer();


	protected:
		virtual void OnDeviceLost();
		virtual void OnDeviceReset();
		virtual void OnDeviceCreate();

		virtual void OnSetAsCurrentIndexBuffer();
		virtual void OnUnsetAsCurrentIndexBuffer();
		

	};

	struct CALC_RADIAN_VERTEX // Simple Optimization. TODO: Deprecated. Should be improved using transformation
	{
		struct
		{
			VEC X, Y;
			VEC RadianAdditive;
		} Cmp;
		VEC Theta;
		VEC Distance;
		VEC Cos, Sin;

		void CmpSet(VEC X, VEC Y, VEC RadianAdditive);
	};

	class QuadDrawer
	{
	public:
	private:
	protected:
		RECTANGLE iRectDest;
		RECTANGLE iRectSource;

		VECTOR2 Size;

	public:
		VERTEX_TEXTURE	Vertex;
		CALC_RADIAN_VERTEX	VertexRadianCalc[4];
		VEC			Radian;
		COLOR		ColorDiffuse;

		TEXTUREADDRESS TextureAddress;

		RECTANGLE	RectDest;
		RECTANGLE	RectSource;
		VECTOR2	Origin;
		VECTOR2	OriginRotation;

		Effect	*Effect;

		bool IsVisible;
		bool IsRotateByOrigin;

		IDirect3DTexture9 *gTexture;

		inline QuadDrawer()
		{
			Size.X = 1;
			Size.Y = 1;

			IsVisible = true;
			IsRotateByOrigin = false;

			Radian = 0;

			ColorDiffuse = 0xffffffff;

			TextureAddress = TEXTUREADDRESS_INHERIT;

			memset(&Vertex, 0, sizeof(Vertex));
			memset(&VertexRadianCalc, 0, sizeof(VertexRadianCalc));

			memset(&RectDest, 0, sizeof(RectDest));
			memset(&RectSource, 0, sizeof(RectSource));
			memset(&iRectDest, 0, sizeof(iRectDest));
			memset(&iRectSource, 0, sizeof(iRectSource));
			memset(&Size, 0, sizeof(Size));

			memset(&Origin, 0, sizeof(Origin));
			memset(&OriginRotation, 0, sizeof(Origin));

			Effect = 0;
			gTexture = 0;
		}

		void Init(VEC Width, VEC Height, IDirect3DTexture9 *gTexture);
		void Init(VEC Width, VEC Height);
		void Init(TextureStatic *Texture);

		void Uninit();

		void Draw();

		void Set(); // Default

		void ApplyColorDiffuse();
		void SetVertex();
		void SetVertex(int ImageWidth, int ImageHeight);

		void SetEffectQuadParameter();
	};

	class LineDrawer // optimized for statical usage
	{
	public:
		struct LINE_PROPERTY
		{
			bool IsPolygonPrev; // cur.V1 is obtained from prev.V2
			bool IsDefaultColor;
			COLOR Color1, Color2;
			VEC X1;
			VEC Y1;
			VEC X2;
			VEC Y2;
		};

	private:
	public:
		//Queue<VERTEX_LINE> QueueLine;
		//Queue<CALC_RADIAN_VERTEX> QueueCalc;
		//Queue<LINE_PROPERTY> QueueLineProperty;
		std::vector<VERTEX_LINE> ArrLine;
		std::vector<LINE_PROPERTY> ArrLineProperty;
		
		bool IsVisible;
		COLOR DefaultColor;

		VECTOR2 Position;
		VECTOR2 Origin;
		VECTOR2 OriginRotation;
		VEC Radian;

		inline LineDrawer()
		{
			DefaultColor = 0xffffffff;
			IsVisible = true;
		
			Radian = Origin.Y = Origin.X = Position.Y = Position.X = 0;
			OriginRotation = Origin;
		}

		void Clear();

		void Draw();

		LINE_PROPERTY* operator [](int Index);

		void AddLine(VEC X1, VEC Y1, VEC X2, VEC Y2, COLOR ColorDiffuse1, COLOR ColorDiffuse2, bool IsDefaultColor, bool IsPolygonPrev);
	};
	
	class RectBorderDrawer
	{
	private:
		PROPERTY_PROVIDE(RectBorderDrawer);

		DECLARE_PROP_TYPE_W(RectBorderDrawer, COLOR, ColorDiffuse,
			{ return ColorDiffuse.Value; },
			{
				ColorDiffuse.Value = BorderQuad[3].ColorDiffuse = BorderQuad[2].ColorDiffuse = BorderQuad[1].ColorDiffuse = BorderQuad[0].ColorDiffuse = Value;
				BorderQuad[0].ApplyColorDiffuse();
				BorderQuad[1].ApplyColorDiffuse();
				BorderQuad[2].ApplyColorDiffuse();
				BorderQuad[3].ApplyColorDiffuse();
			});
		DECLARE_PROP_TYPE_RW(RectBorderDrawer, VEC, Padding, { return Padding.Value; }, { Padding.Value = Value; ApplyShape(); });
		DECLARE_PROP_TYPE_RW(RectBorderDrawer, RECTANGLE, Position, { return Position.Value; }, { Position.Value = Value; ApplyShape(); });

	public:
		DECLARE_PROPERTY(ColorDiffuse);
		DECLARE_PROPERTY(Padding);
		DECLARE_PROPERTY(Position);

		QuadDrawer BorderQuad[4];

		inline RectBorderDrawer()
			: ColorDiffuse(new COLOR(0xFFFFFFFF)),
			Padding(new VEC(4)),
			Position(new RECTANGLE(0, 0, 100, 100))
		{ }

		void Draw();

		void ApplyShape();
	};


	typedef class TextureStatic
		: public IDeviceResource
	{
		PROPERTY_PROVIDE(TextureStatic);
	public:
		struct INIT_PARAM
		{
			bool IsRenderTarget;
			bool IsInteractive;
			bool IsReliable;
		};

		struct RESTORE_METHOD
		{
			enum RESTORE_TYPE
			{
				FromFile = 0,
				FillColor
			};
			RESTORE_TYPE Type;

			struct FROMFILE
			{
				Util::UnwindArray<wchar_t> Path;
			} FromFileType;
			struct FILLCOLOR
			{
				COLOR Color;
				SIZE Size;
			} FillColorType;
		
			void SetFromFile(wchar_t *FileName);
			void SetColorFill(COLOR Color, SIZE Size);
		}; 

	protected:
		RESTORE_METHOD RestoreMethod;

		LPDIRECT3DTEXTURE9 gTexture;
		LPDIRECT3DSURFACE9 gSurface;

		D3DSURFACE_DESC SurfaceDesc;
		D3DXIMAGE_INFO  ImageDesc;

		DECLARE_PROP_TYPE_R(TextureStatic, INIT_PARAM, SelectedInitParam, { return SelectedInitParam.Value; }, { SelectedInitParam.Value = Value; });

		virtual void InitPost();

	public:
		INIT_PARAM InitParam;
		DECLARE_PROPERTY(SelectedInitParam);

		//QuadDrawer Drawer;

		int ListIndex;

		inline TextureStatic()
		{
			__SetResourceType(IDeviceResource::TEXTURE);

			InitParam.IsRenderTarget = false;
			InitParam.IsInteractive = false;
			InitParam.IsReliable = true;

			gTexture = 0;
			gSurface = 0;

			memset(&SurfaceDesc, 0, sizeof(SurfaceDesc));
			memset(&ImageDesc, 0, sizeof(ImageDesc));

			ListIndex = DeviceResourceManager::ListTexture().Add(this);
		}
		virtual ~TextureStatic();


		// Inits and fills the texture content with the image file
		virtual void Init(wchar_t *FileName);
		// Inits with the size and fills with the color.
		virtual void Init(COLOR Color, DWORD Width, DWORD Height);


		virtual void Uninit();

		virtual IDirect3DTexture9 *GetTexture();
		virtual IDirect3DSurface9 *GetSurface();
		virtual UINT GetWidth();
		virtual UINT GetHeight();

	protected:
		virtual void OnDeviceLost();
		virtual void OnDeviceReset();
		virtual void OnDeviceCreate();

	public:
		Event EventPrepareContent;

	} Texture;

	class RenderTarget
		: public TextureStatic
	{
	public:
		inline RenderTarget()
		{
			InitParam.IsRenderTarget = true;
			InitParam.IsInteractive = false;
			InitParam.IsReliable = false;
		}
	};

	class TextureDynamic
		: public TextureStatic
	{
	public:
		inline TextureDynamic()
		{
			InitParam.IsRenderTarget = false;
			InitParam.IsInteractive = true;
			InitParam.IsReliable = false;
		}
	};

	class Effect
		: public IDeviceResource
	{
	private:
		static char *BaseShaderFileBuffer;
		static DWORD BaseShaderFileBufferSize;

		void InitShader(char *FunctionName, bool IsVertexShader);

		PROPERTY_PROVIDE(Effect);
		DECLARE_PROP_TYPE_R(Effect, bool, IsPixelShaderLoaded, { return IsPixelShaderLoaded.Value; }, { IsPixelShaderLoaded.Value = Value; } );
		DECLARE_PROP_TYPE_R(Effect, bool, IsVertexShaderLoaded, { return IsVertexShaderLoaded.Value; }, { IsVertexShaderLoaded.Value = Value; } );
		

	public:
		class EffectConstant
		{
		private:
		public:
			Util::Queue<EFFECT_CONSTANT*> ConstantTable;

			inline EffectConstant()
			{ }
			~EffectConstant();

			void AddConstant(char *Ident, EFFECT_VALUE_TYPE Type, void *Data, DWORD Cb);
		};

		int ListIndex;
		DECLARE_PROPERTY(IsPixelShaderLoaded);
		DECLARE_PROPERTY(IsVertexShaderLoaded);
		bool IsEnabled;		//Decides that shader will apply
		LPD3DXCONSTANTTABLE gConstantTable;

		EffectConstant ConstantSaved;
		LPD3DXBUFFER gCode;
		IDirect3DPixelShader9 *gPixelShader;
		IDirect3DVertexShader9 *gVertexShader;

		double Progress, MaxProgress;
		int LoopCount;
		float MultipleProgress;
		bool IsStarted;

		inline Effect()
		{
			__SetResourceType(IDeviceResource::EFFECT);
			IsVertexShaderLoaded = IsPixelShaderLoaded = false;
			IsEnabled = true;

			gConstantTable = NULL;
			gPixelShader = NULL;
			gVertexShader = NULL;
			ListIndex = DeviceResourceManager::ListEffect().Add(this);

			LoopCount = 0;
			Progress = 0;
			IsStarted = 0;
			MaxProgress = 1;
			MultipleProgress = 1;
		}

		virtual ~Effect();
		Effect(wchar_t *FileName);
		Effect(void *Content);

		/*bool InitFromFile(wchar_t *FileName, char *FunctionName);
		bool InitFromMemory(void *Content);*/
		virtual void InitPixelShader(char *FunctionName);
		virtual void InitVertexShader(char *FunctionName);

		virtual void Uninit();

		virtual void ReloadConstant();

		virtual void SetParameter(char *ParamName, EFFECT_VALUE_TYPE Type, void *Value, DWORD Size);
		virtual void SetParameter(char *ParamName, float Value);
		virtual void SetParameter(char *ParamName, int Value);
		virtual void SetParameter(char *ParamName, LPDIRECT3DBASETEXTURE9 Texture);

		virtual void Start(int MaxProgress, float MultipleProgress, int LoopCount);
		virtual void Stop();

		virtual void Update(double ElapsedTime);

		virtual void Apply();
		virtual void SetDefaultParameter();

		static bool InitBaseShaderFromFile(wchar_t *FileName);
		static bool InitBaseShaderFromMemory(void *Mem, DWORD Size);
		static void UninitBaseShader();

	protected:
		virtual void OnSetAsCurrentEffect();
		virtual void OnUnsetAsCurrentEffect();

		virtual void OnDeviceLost();
		virtual void OnDeviceReset();
		virtual void OnDeviceCreate();

	};


	class Font
		: public IDeviceResource
	{
	private:
	public:
		bool IsItalic;
		float Z[4];
		D3DXFONT_DESC FontDesc;
		ID3DXFont *gFont;

		DWORD FontIndex;


		inline Font()
		{
			__SetResourceType(IDeviceResource::FONT);

			gFont = 0;
			//__SetIsDeviceLost(false);
			__SetIsInited(false);
			IsItalic = false;
			Z[0] = 0.0f;
			Z[1] = 0.0f;
			Z[2] = 0.0f;
			Z[3] = 0.0f;
			memset(&FontDesc, 0, sizeof(FontDesc));
			FontIndex = DeviceResourceManager::ListFont().Add(this);
		}
		virtual ~Font();

		virtual void Init(wchar_t *FaceName, DWORD Width, int Height, DWORD Weight, bool IsClearType);
		virtual void Uninit();

		// TODO: Cache, so optimize
		virtual SIZE Draw(const std::wstring & DrawStr, int X, int Y, int Width, int Height, COLOR Color, DWORD Flags, bool IsUsingZBuffer);
		virtual RECT GetTextDrawRect(const std::wstring& DrawStr, int X, int Y, int Width, int Height, COLOR Color, DWORD Flags);

	protected:
		virtual void OnDeviceLost();
		virtual void OnDeviceReset();
		virtual void OnDeviceCreate();
	};

	class Camera2D
		: public IDeviceResource
	{
	private:
		PROPERTY_PROVIDE(Camera2D);
		
		DECLARE_PROP_TYPE_RW(Camera2D, VECTOR2, OriginRotate,
			{ return OriginRotate.Value; },
			{
				OriginRotate.Value = Value;
				SetMatrix();
			} );
		DECLARE_PROP_TYPE_RW(Camera2D, RECTANGLE, Position,
			{ return Position.Value; },
			{
				Position.Value = Value; 
				SetMatrix();
			} );
		DECLARE_PROP_TYPE_RW(Camera2D, VEC, Radian,
			{ return Radian.Value; },
			{
				Radian.Value = Value;
				SetMatrix();
			} );
		

	protected:
		D3DXMATRIXA16 MatrixTransform;
		void SetMatrix();
		
	public:
		DECLARE_PROPERTY(OriginRotate);
		DECLARE_PROPERTY(Position);
		DECLARE_PROPERTY(Radian);

		inline Camera2D(){ Uninit(); }
		virtual ~Camera2D();

		void Init();
		void Init(DWORD Width, DWORD Height);
		void Uninit();

		//void Draw();
		void TransformScreenCoord(VERTEX_DOT & Vertex);
		void TransformScreenCoord(VERTEX_LINE & Line);
		void TransformScreenCoord(VERTEX_TRIANGLE & Triangle);
		void TransformScreenCoord(VERTEX_TEXTURE & Quad);
		const D3DXMATRIXA16 & GetMatrixTransform();
	protected:
		virtual void OnDeviceLost();
		virtual void OnDeviceReset();
		virtual void OnDeviceCreate();
	};

	struct SAMPLER_STATE
	{
		enum TEMPLATE_TYPE
		{
			DEFAULT = 0,
			BORDER,
			CLAMP,
			WRAP,
		};
		D3DTEXTUREADDRESS TextureAddress;

		D3DCOLOR	BorderColor;

		//D3DTEXTUREFILTERTYPE	MagnificationFilter; // D3DTEXF_POINT
		//D3DTEXTUREFILTERTYPE	MinificationFilter; // D3DTEXF_POINT
		//D3DTEXTUREFILTERTYPE	MipMinFilter; // D3DTEXF_NONE
		D3DTEXTUREFILTERTYPE Filter;
		//float	MipMapLODBias;

		//DWORD	MaxMipLevel; // 0..(n-1) LOD index of largest map to use (0 == largest)
		//DWORD	MaxAnisotropy;

		//bool IsSRGBTexture; // Gamma related

		//int ElementIndex; /* When multi-element texture is assigned to sampler, this indicates which element index to use.  Default = 0.  */
		//int DisplacementMapOffset; D3DSAMP_DMAPOFFSET     = 13, /* Offset in vertices in the pre-sampled displacement map. Only valid for D3DDMAPSAMPLER sampler  */
		//	D3DSAMP_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */

		inline SAMPLER_STATE()
		{ Template(DEFAULT); }
		inline SAMPLER_STATE(TEMPLATE_TYPE Type)
		{ Template(Type); }

		inline void Template(TEMPLATE_TYPE Type)
		{
			BorderColor = 0x00000000;
			Filter = D3DTEXF_ANISOTROPIC;

			if ( Type == DEFAULT ) Type = WRAP;

			switch ( Type )
			{
			case BORDER:
				TextureAddress = D3DTADDRESS_BORDER;
				break;
			case CLAMP:
				TextureAddress = D3DTADDRESS_CLAMP;
				break;
			case WRAP:
				TextureAddress = D3DTADDRESS_WRAP;
				break;
			default: 
				NRTHROW();
				break;
			}
		}

		void Apply();
	};

	struct TEXTURE_STAGE_STATE
	{
		enum TEMPLATE_TYPE
		{
			DEFAULT,
			MODULATE,
			ADD,
		};
		enum TA
		{
			SELECTMASK = D3DTA_SELECTMASK,		// mask for arg selector
			DIFFUSE = D3DTA_DIFFUSE,			// select diffuse color (read only)
			CURRENT = D3DTA_CURRENT,			// select stage destination register (read/write)
			TEXTURE = D3DTA_TEXTURE,			// select texture color (read only)
			TFACTOR = D3DTA_TFACTOR,			// select D3DRS_TEXTUREFACTOR (read only)
			SPECULAR = D3DTA_SPECULAR,			// select specular color (read only)
			TEMP = D3DTA_TEMP,					// select temporary register color (read/write)
			CONSTANT = D3DTA_CONSTANT,			// select texture stage constant
			COMPLEMENT = D3DTA_COMPLEMENT,		// take 1.0 - x (read modifier)
			ALPHAREPLICATE = D3DTA_ALPHAREPLICATE	// replicate alpha to color components (read modifier)
		};
		struct TEXTURESTAGE_FUNCTION
		{
			TA LeftArg;
			D3DTEXTUREOP Operator;
			TA RightArg;

			inline bool operator==(TEXTURESTAGE_FUNCTION const & A) const
			{ return 0 == memcmp(&A, this, sizeof(*this)); }
			inline bool operator!=(TEXTURESTAGE_FUNCTION const & A) const
			{ return 0 != memcmp(&A, this, sizeof(*this)); }

			inline TEXTURESTAGE_FUNCTION()
			{
				// 스테이지를 0,1,2,3 이렇게 거치면서 각각 적용된다.
				// Current 는 0일땐 DIFFUSE, 1,2,3일땐 스테이지의 각 결과란다
				LeftArg = TEXTURE;
				Operator = D3DTOP_MODULATE;
				RightArg = CURRENT;
			}
		};

		TEXTURESTAGE_FUNCTION ColorFunc;
		TEXTURESTAGE_FUNCTION AlphaFunc;

		inline TEXTURE_STAGE_STATE(TEMPLATE_TYPE Type)
		{ Template(Type); }
		inline TEXTURE_STAGE_STATE()
		{ Template(DEFAULT); }

		inline void Template(TEMPLATE_TYPE Type)
		{
			switch ( Type )
			{
			case DEFAULT:
				Template(MODULATE);
				break;
			case MODULATE:
				ColorFunc.Operator = D3DTOP_MODULATE;
				AlphaFunc.Operator = D3DTOP_MODULATE;
				break;
			case ADD:
				ColorFunc.Operator = D3DTOP_ADD;
				AlphaFunc.Operator = D3DTOP_ADD;
				break;
			default:
				NRTHROW();
				break;
			}
		}

		void Apply();
	};

	// It will be a little faster when blending and alphablending is equal to each other
	struct BLEND_STATE
	{
		enum TEMPLATE_TYPE
		{
			OVERLAY,
			DEFAULT = OVERLAY,
			COPY,
			ALPHA_STENCIL_SETTING,
			DRAW_NOTHING,

		};

		// result = Src*(SrcAlpha) operator Dest*(DestAlpha)
		struct BLENDING_FUNCTION
		{
			D3DBLEND OperandSrcMul;
			D3DBLENDOP Operator;
			D3DBLEND OperandDestMul;

			inline bool operator==(BLENDING_FUNCTION const & A) const
			{ return 0 == memcmp(&A, this, sizeof(*this)); }
			inline bool operator!=(BLENDING_FUNCTION const & A) const
			{ return 0 != memcmp(&A, this, sizeof(*this)); }
		};
		// If CmpFunc (of value) is true, draw that
		struct ALPHA_PASS_CONDITION_FUNCTION
		{
			D3DCMPFUNC AllowIf;
			DWORD ReferenceValue;
		};

		bool IsAlphablendable;

		//bool IsSeparateAlphablendEnable; // automatic

		BLENDING_FUNCTION ColorFunc;
		BLENDING_FUNCTION AlphaFunc;

		ALPHA_PASS_CONDITION_FUNCTION AlphaPassConditionFunction;

		inline BLEND_STATE()
		{ Template(DEFAULT); }
		inline BLEND_STATE(TEMPLATE_TYPE Type)
		{ Template(Type); }

		inline void Template(TEMPLATE_TYPE Type)
		{
			IsAlphablendable = true;

			switch (Type)
			{
			case OVERLAY:
				// 				SetRawRenderState(D3DRS_ALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				// 				SetRawRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				// 				SetRawRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				// 				SetRawRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
				// 				SetRawRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
				// 				SetRawRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);

				ColorFunc.OperandSrcMul = D3DBLEND_SRCALPHA;
				ColorFunc.Operator = D3DBLENDOP_ADD;
				ColorFunc.OperandDestMul = D3DBLEND_INVSRCALPHA;

				AlphaFunc.OperandSrcMul = D3DBLEND_ONE;
				AlphaFunc.Operator = D3DBLENDOP_ADD;
				AlphaFunc.OperandDestMul = D3DBLEND_INVSRCALPHA;
				break;

			case COPY:
				// 				SetRawRenderState(D3DRS_ALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_BLENDOP,  D3DBLENDOP_ADD);
				// 				SetRawRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				// 				SetRawRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				// 				SetRawRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
				// 				SetRawRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
				// 				SetRawRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);

				ColorFunc.OperandSrcMul = D3DBLEND_ONE;
				ColorFunc.Operator = D3DBLENDOP_ADD;
				ColorFunc.OperandDestMul = D3DBLEND_INVSRCALPHA;

				AlphaFunc.OperandSrcMul = D3DBLEND_ONE;
				AlphaFunc.Operator = D3DBLENDOP_ADD;
				AlphaFunc.OperandDestMul = D3DBLEND_ZERO;
				break;

			case ALPHA_STENCIL_SETTING:
				// 				SetRawRenderState(D3DRS_ALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
				// 				SetRawRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				// 				SetRawRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_DESTALPHA);
				// 				SetRawRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
				// 				SetRawRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
				// 				SetRawRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

				ColorFunc.OperandSrcMul = D3DBLEND_ONE;
				ColorFunc.Operator = D3DBLENDOP_ADD;
				ColorFunc.OperandDestMul = D3DBLEND_ZERO;

				AlphaFunc.OperandSrcMul = D3DBLEND_DESTALPHA;
				AlphaFunc.Operator = D3DBLENDOP_ADD;
				AlphaFunc.OperandDestMul = D3DBLEND_ZERO;
				break;
			case DRAW_NOTHING:
				ColorFunc.OperandSrcMul = D3DBLEND_ZERO;
				ColorFunc.Operator = D3DBLENDOP_ADD;
				ColorFunc.OperandDestMul = D3DBLEND_ONE;
				AlphaFunc = ColorFunc;
				break;
			default:
				NRTHROW();
				break;
			}
		}

		void Apply();
	};

	struct STENCIL_STATE
	{
		enum TEMPLATE_TYPE
		{
			DISABLED,
			DEFAULT = DISABLED,
			STENCIL_SETTING,
			STENCIL_MASKING
		};

		// If CmpFunc (of value) is true, pass(draw) that
		struct PASS_CONDITION_FUNCTION
		{
			D3DCMPFUNC PassIf;
			DWORD ReferenceValue;

			inline PASS_CONDITION_FUNCTION()
			{
				PassIf = D3DCMP_ALWAYS;
				ReferenceValue = 0;
			}
		};

		bool IsStencilEnable;
		PASS_CONDITION_FUNCTION StencilPassConditionFunction;

		//D3DSTENCILOP ZFailOperation;
		D3DSTENCILOP FailOperation;
		D3DSTENCILOP PassOperation;

		DWORD CmpMask, WriteMask;

		inline STENCIL_STATE()
		{ Template(DEFAULT); }
		inline STENCIL_STATE(TEMPLATE_TYPE Type)
		{ Template(Type); }

		inline void Template(TEMPLATE_TYPE Type)
		{
			IsStencilEnable = false;

			FailOperation = D3DSTENCILOP_KEEP;
			PassOperation = D3DSTENCILOP_KEEP;

			CmpMask = 0xffffffff;
			WriteMask = 0xffffffff;

			switch ( Type )
			{
			case DISABLED:
				break;
			case STENCIL_SETTING:
				IsStencilEnable = true;
				StencilPassConditionFunction.PassIf = D3DCMP_ALWAYS;
				StencilPassConditionFunction.ReferenceValue = 0x1;
				FailOperation = D3DSTENCILOP_KEEP;
				PassOperation = D3DSTENCILOP_REPLACE;
				break;
			case STENCIL_MASKING:
				IsStencilEnable = true;
				StencilPassConditionFunction.PassIf = D3DCMP_EQUAL;
				StencilPassConditionFunction.ReferenceValue = 0x1;
				FailOperation = D3DSTENCILOP_KEEP;
				PassOperation = D3DSTENCILOP_REPLACE;
				break;
			default:
				NRTHROW();
				break;
			}
		}

		void Apply();
	};

	class BatchBase;
	class BatchScope;

	class BatchBase
	{
	private:
		PROPERTY_PROVIDE(BatchBase);

	protected:
		DECLARE_PROP_TYPE_R(BatchBase, bool, IsEnabled, { return IsEnabled.Value; }, { IsEnabled.Value = Value; });

		virtual void OnBegin();
		virtual void OnEnd();

	public:
		DECLARE_PROPERTY(IsEnabled); 

		virtual void Begin();
		virtual BatchScope BeginScope();
		virtual void End();
	};

	class BlendStateBatch : public BatchBase
	{
		
	};

	class BatchScope
	{
	private:
		PROPERTY_PROVIDE(BatchScope);

	protected:
		DECLARE_PROP_TYPE_R(BatchScope, BatchBase*, Owner, { return Owner.Value; }, { Owner.Value = Value; });

	public:
		DECLARE_PROPERTY(Owner);
		
		inline BatchScope(decltype(Owner) Owner)
		{
			ATHROW(Owner);
			this->Owner = Owner;
		}
		inline BatchScope(BatchScope & Copy)
		{
			this->Owner = Copy.Owner; // ownership transfer
			Copy.Owner = NULL;
		}

		inline ~BatchScope()
		{ if ( Owner ) Owner->End(); }
	};

	class ReverterTexture
	{
	public:
		TextureStatic *OriginalTexture;
		ReverterTexture();
		~ReverterTexture();
		void Restore();
	};

	class ReverterRawTexture
	{
	public:
		IDirect3DBaseTexture9 *ArrOriginalTexture[8];

		ReverterRawTexture();
		~ReverterRawTexture();
		void Restore();
	};

	class ReverterRenderTarget
	{
	public:
		TextureStatic *OriginalRenderTarget;
		ReverterRenderTarget();
		~ReverterRenderTarget();
		void Restore();
	};

	class ReverterRawRenderTarget
	{
	public:
		//int CountRenderTarget;
		//IDirect3DSurface9 *ArrOriginalRenderTarget[16];
		//UnwindArray<IDirect3DSurface9 *> uwArrOriginalRenderTarget;
		IDirect3DSurface9 *OriginalRenderTarget;

		ReverterRawRenderTarget();
		~ReverterRawRenderTarget();
		void Restore();
	};

	class ReverterEffect
	{
	public:
		Effect *OriginalEffect;
		ReverterEffect();
		~ReverterEffect();
		void Restore();
	};

	class ReverterRawPixelShader
	{
	public:
		IDirect3DPixelShader9 *OriginalPixelShader;
		ReverterRawPixelShader();
		~ReverterRawPixelShader();
		void Restore();
	};

	class ReverterRawVertexShader
	{
	public:
		IDirect3DVertexShader9 *OriginalVertexShader;
		ReverterRawVertexShader();
		~ReverterRawVertexShader();
		void Restore();
	};

	class ReverterBlendState
	{
	public:
		BLEND_STATE OriginalBlendState;
		ReverterBlendState();
		~ReverterBlendState();
		void Restore();
	};

	class ReverterRawBlendState
	{
	public:
		DWORD OriginalSrcBlend,
			OriginalSrcBlendAlpha,
			OriginalDestBlend,
			OriginalDestBlendAlpha,
			OriginalAlphaBlendable,
			OriginalBlendOp,
			OriginalColorWriteEnable,
			OriginalSeparateAlphaBlendEnable;

		ReverterRawBlendState();
		~ReverterRawBlendState();
		void Restore();
	};

	class ReverterRawRenderState
	{
	public:
		DWORD Original[256];

		ReverterRawRenderState();
		~ReverterRawRenderState();
		void Restore();
	};

	class ReverterStencilState
	{
	public:
		STENCIL_STATE OriginalStencilState;
		ReverterStencilState();
		~ReverterStencilState();
		void Restore();
	};

	class ReverterRawStencilState
	{
	public:
		DWORD OriginalStencilEnable,
			OriginalStencilFail,
			OriginalStencilFunc,
			OriginalStencilMask,
			OriginalStencilPass,
			OriginalStencilRef,
			OriginalStencilWriteMask,
			OriginalStencilZFail,
			OriginalZEnable,
			OriginalZFunc,
			OriginalZWriteEnable;

		ReverterRawStencilState();
		~ReverterRawStencilState();

		void Restore();
	};

	class ReverterSamplerState
	{
	public:
		SAMPLER_STATE OriginalSamplerState;
		ReverterSamplerState();
		~ReverterSamplerState();
		void Restore();
	};

	class ReverterRawSamplerState
	{
	public:
		struct ORIGINAL
		{
			DWORD AddressU,
				AddressV,
				BorderColor,
				MipFilter,
				MagFilter,
				MinFilter,
				MipMapLodBias,
				MaxMipLevel,
				MaxAnisotropy,
				SrgbTexture,
				ElementIndex,
				DMapOffset;
		} Original[8];

		ReverterRawSamplerState();
		~ReverterRawSamplerState();
		void Restore();
	};

	class ReverterTextureStageState
	{
		TEXTURE_STAGE_STATE OriginalTextureStageState;
		ReverterTextureStageState();
		~ReverterTextureStageState();
		void Restore();
	};

	class ReverterRawTextureStageState
	{
	public:
		struct ORIGINAL
		{
			DWORD 
				ALPHAARG0,
				ALPHAARG1,
				ALPHAARG2,
				ALPHAOP,
				BUMPENVLOFFSET,
				BUMPENVLSCALE,
				BUMPENVMAT00,
				BUMPENVMAT01,
				BUMPENVMAT10,
				BUMPENVMAT11,
				COLORARG0,
				COLORARG1,
				COLORARG2,
				COLOROP,
				CONSTANT,
				//FORCE_DWORD,
				RESULTARG,
				TEXCOORDINDEX,
				TEXTURETRANSFORMFLAGS;
		} Original[8];

		ReverterRawTextureStageState();
		~ReverterRawTextureStageState();
		void Restore();
	};

	class ReverterIndexBuffer
	{
	public:
		IndexBuffer *OriginalIndexBuffer;

		ReverterIndexBuffer();
		~ReverterIndexBuffer();
		void Restore();
	};

	class ReverterRawIndexBuffer
	{
	public:
		IDirect3DIndexBuffer9 *OriginalIndexBuffer;

		ReverterRawIndexBuffer();
		~ReverterRawIndexBuffer();
		void Restore();
	};

	class ReverterRawDepthStencilSurface
	{
	public:
		IDirect3DSurface9 *OriginalDepthStencilSurface;

		ReverterRawDepthStencilSurface();
		~ReverterRawDepthStencilSurface();
		void Restore();
	};



}