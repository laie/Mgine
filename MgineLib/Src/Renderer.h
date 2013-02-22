#pragma once

//#define MGINE_RENDERER_UNIT_SIZE		16384
//#define MGINE_DEFAULT_FVF				(D3DFVF_DIFFUSE | D3DFVF_XYZ/*RHW*/ | D3DFVF_TEX1 )

#include "DeviceResource.h"
namespace Mgine
{
	class Renderer
	{
	public:
		struct STATE_TEXTUREBATCH;
		struct STATE_INDEXBUFFER;
		struct STATE_STENCILBUFFER;
		struct STATE_ALPHASTENCIL;
		struct STATE_STENCIL;
		struct STATE_CACHE;
		struct STATE_RENDERER;

		static const int PrimUnitCount = 16384;
		static const DWORD DefaultFVF = (D3DFVF_DIFFUSE | D3DFVF_XYZ/*RHW*/ | D3DFVF_TEX1);

	private:
		static bool IsBlur;

		static bool IsInited;
		static bool IsDeviceLost;

		static D3DDISPLAYMODE DisplayMode;
		static D3DCAPS9 DeviceCaps;


		static VERTEX_DOT	DrawArray[PrimUnitCount];
		static DWORD		CurrentDrawIndex;

		static PRIM_TYPE			CurrentPrimType;

		static IDirect3DVertexBuffer9	*gVertexBuffer;
		static IDirect3DIndexBuffer9	*gIndexBuffer;

		static LPD3DXLINE				gLineBatch;


		static Effect				EffectBlur;

		static VEC					ProjectionOffsetX; // Offset for texture pixel matching
		static VEC					ProjectionOffsetY; // Offset for texture pixel matching
		static VEC					PrimOffsetX;
		static VEC					PrimOffsetY;

		static D3DVIEWPORT9			Viewport;
		static DWORD				ResolutionWidth;
		static DWORD				ResolutionHeight;

		static D3DXMATRIX MatrixWorld;
		static D3DXMATRIX MatrixView;
		static D3DXMATRIX MatrixProjection;

		static IDirect3DSurface9 *BackBuffer;
		static IDirect3DSurface9 *DefaultDepthStencilSurface;

		static void LoadBlur();
		static void UnloadBlur();

		#pragma region Sub procedures
		static void SubOnDeviceCreateLine();
		static void SubOnDeviceCreateIndexBuffer();
		static void SubOnDeviceCreateVertexBuffer();
		#pragma endregion
	public:
		/*
			0~1 : Texture Effect
			0	: Font
			2	: Alpha Stencil Mask	Begin~End
			3	: Alpha Stencil Drawn	Begin~End
		*/
		static RenderTarget TemporaryRenderTargetTexture[4];
		static RenderTarget TemporaryRenderTargetMovingScene[2];
		static RenderTarget TemporaryRenderTargetBlur[1];

		static STATE_TEXTUREBATCH	StateTextureBatch;
		static STATE_INDEXBUFFER	StateIndexBuffer;
		static STATE_STENCILBUFFER	StateStencilBuffer;
		static STATE_ALPHASTENCIL	StateAlphaStencil;
		static STATE_CACHE			StateCache;
		static STATE_RENDERER		StateRenderer;

		static void Init();
		static void Uninit();

		static void OnDeviceCreate();
		static void OnDeviceLost();
		static void OnDeviceReset();
		static void OnDeviceStart();

		/*\
		 *	Getters
		\*/
		static DWORD GetResolutionWidth();
		static DWORD GetResolutionHeight();

		static IDirect3DSurface9 *GetRawBackBuffer();
		static IDirect3DSurface9 *GetRawDefaultDepthStencilSurface();

		static D3DDISPLAYMODE const & GetRawDisplayMode();
		static D3DCAPS9 const & GetRawDeviceCaps();

		static D3DXMATRIX const & GetMatrixWorld();
		static D3DXMATRIX const & GetMatrixView();
		static D3DXMATRIX const & GetMatrixProjection();

		/*\
		 *	Raw Renderer Control Methods
		\*/
		static IDirect3DBaseTexture9 *GetRawTexture();
		static IDirect3DBaseTexture9 *GetRawTexture(int Stage);
		static DWORD GetRawTextureStageState(D3DTEXTURESTAGESTATETYPE Type);
		static DWORD GetRawTextureStageState(int Stage, D3DTEXTURESTAGESTATETYPE Type);
		static IDirect3DIndexBuffer9 *GetRawIndexBuffer();
		static IDirect3DSurface9 *GetRawRenderTarget();
		static IDirect3DPixelShader9 *GetRawPixelShader();
		static IDirect3DVertexShader9 *GetRawVertexShader();
		static DWORD GetRawRenderState(D3DRENDERSTATETYPE Type);
		static DWORD GetRawSamplerState(int Sampler, D3DSAMPLERSTATETYPE Type);
		static IDirect3DSurface9 *GetRawDepthStencilSurface();

		static void SetRawTexture(IDirect3DBaseTexture9 *Texture);
		static void SetRawTexture(int Stage, IDirect3DBaseTexture9 *Texture);
		static void SetRawRenderTarget(IDirect3DSurface9 *Surface);
		//static bool SetRenderTarget(int Index, IDirect3DSurface9 *Surface);
		static void SetRawPixelShader(IDirect3DPixelShader9 *PixelShader);
		static void SetRawVertexShader(IDirect3DVertexShader9 *VertexShader);
		static void SetRawRenderState(D3DRENDERSTATETYPE Type, DWORD Value);
		static void ResetRawSamplerState();
		static void SetRawSamplerState(D3DSAMPLERSTATETYPE Type, DWORD Value);
		static void SetRawSamplerState(int Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
		static void SetRawTextureStageState(D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
		static void SetRawTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
		static void SetProjection(DWORD Width, DWORD Height);
		static void SetRawIndexBuffer(IDirect3DIndexBuffer9 *IndexBuffer);
		static void SetRawDepthStencilSurface(IDirect3DSurface9 *DepthStencilSurface);


		
		/*\
		 *	Framework Renderer Control Methods
		\*/

		static TextureStatic *GetRenderTarget();
		static TextureStatic *GetTexture();
		static const SAMPLER_STATE& GetSamplerState();
		static const TEXTURE_STAGE_STATE& GetTextureStageState();
		static const BLEND_STATE& GetBlendState();
		static const STENCIL_STATE& GetStencilState();
		static IndexBuffer *GetIndexBuffer();
		static Effect *GetEffect();
		static Camera2D *GetCamera();

		static void SetRenderTarget(TextureStatic *RenderTarget);
		static void SetTexture(TextureStatic *Texture);
		static void SetSamplerState(const SAMPLER_STATE & SamplerState);
		static void SetTextureStageState(const TEXTURE_STAGE_STATE & TextureStageState);
		static void SetBlendState(const BLEND_STATE & BlendState);
		static void SetStencilState(const STENCIL_STATE & StencilState);
		static void SetIndexBuffer(IndexBuffer *IndexBufferParam);
		static void SetEffect(Effect *EffectParam);
		static void SetCamera(Camera2D *Camera);

		static void BeginTextureBatch(TextureStatic *TextureParam);
		static void EndTextureBatch();
		
		static void BeginIndexBuffer(IndexBuffer *IndexBufferParam);
		static void EndIndexBuffer();
		
		static void BeginStencilSet(bool IsClear);
		static void BeginStencilRender();
		static void EndStencil();
		
		static void BeginAlphaStencilSet(bool IsClear);
		static void BeginAlphaStencilRender();
		static void EndAlphaStencil();

		static void PrepareBackBuffer();
		static void PrepareDefaultDepthStencilSurface();

		static void Clear(DWORD Color, bool IsRenderTaget, bool IsStencil, bool IsZBuffer);

		static void DrawQuad(VERTEX_TEXTURE *QuadArray, DWORD QuadCount);
		static void DrawQuad(VERTEX_TEXTURE *QuadArray, DWORD QuadCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection);
		static void DrawTri(VERTEX_TRIANGLE *TriArray, DWORD TriCount);
		static void DrawTri(VERTEX_TRIANGLE *TriArray, DWORD TriCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection);
		static void DrawLine(VERTEX_LINE *LineArray, DWORD LineCount);
		static void DrawLine(VERTEX_LINE *LineArray, DWORD LineCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection);
		static void DrawPoint(VERTEX_DOT *VtArray, DWORD VtCount);
		static void DrawPoint(VERTEX_DOT *VtArray, DWORD VtCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection);
		static void DrawPolygon(D3DXVECTOR2 *Points, int Count, DWORD Color);
		static void FlushBatch(); // Forces draw all the batched primitives

		static void Present(); // Presents the screen drawn on backbuffer

		static void FinishFrame(); // Reset all the rendererstates

		static void Begin(); // Begin of drawing primitives. On drawing methods, it treated automatically so you don't need to call this. However, on other situation like on update methods, Begin() and End() should be called manually.
		static void End(); // End of drawing primitives. See Begin's description for remark.

		static void SetBlur(bool IsEnabling);
		static void ApplyBlur();

		static void TestHalfPixel();



		struct STATE_TEXTUREBATCH
		{
			Util::UnwindItem<ReverterTexture>	RestoreTextureBatch;
			
			void Set(TextureStatic *TextureParam);
			void Uninit();
		};

		struct STATE_INDEXBUFFER
		{
			Util::UnwindItem<ReverterIndexBuffer> RestoreIndexBuffer;
			IndexBuffer *IndexBufferSet;
		
			void Set(IndexBuffer *IndexBufferParam);
			void Uninit();
		};

		struct STATE_STENCILBUFFER
		{
			Util::UnwindItem<ReverterStencilState>			RestoreStencilState;
			Util::UnwindItem<ReverterRawDepthStencilSurface>	RestoreDepthStencilSurface;
			Util::UnwindItem<ReverterBlendState>			RestoreBlendState;

			void SetStencil(bool IsClear);
			void SetRender();
			void Uninit();
		};

		struct STATE_ALPHASTENCIL
		{
			Util::UnwindItem<ReverterRawRenderTarget>	RestoreRenderTarget;
			Util::UnwindItem<ReverterRawBlendState>	RestoreBlendState;

			bool IsMask;

			void SetStencil(bool IsClear);
			void SetRender();
			void Uninit();
		};
		struct STATE_CACHE
		{
			bool IsCachePause;

			IDirect3DBaseTexture9 *Texture[8];
			DWORD SamplerState[8][20];
			DWORD TextureStageState[8][40];

			DWORD RenderState[256];

			IDirect3DIndexBuffer9 *IndexBuffer;
			//UnwindArray<IDirect3DSurface9 *> ArrRenderTarget; // count of NumSimultaneousRTs
			//int CountRenderTarget;

			IDirect3DSurface9 *RenderTarget;

			IDirect3DSurface9 *DepthStencilSurface;

			IDirect3DPixelShader9 *PixelShader;
			IDirect3DVertexShader9 *VertexShader;


			void CacheRenderTarget();
			void CacheSamplerState();
			void CacheRenderState();
			void CacheIndexBuffer();
			void CacheDepthStencilSurface();
			void CacheShader();
			void Cache();
		};
		struct STATE_RENDERER
		{
			SAMPLER_STATE SamplerState;
			TEXTURE_STAGE_STATE TextureStageState;
			BLEND_STATE BlendState;
			STENCIL_STATE StencilState;

			TextureStatic *RenderTarget;
			TextureStatic *Texture;
			IndexBuffer *IndexBuffer;
			//TextureDepthStencil DepthStencilSurface;

			Effect *EffectShader;
			Camera2D *Camera;


		};

	};
}