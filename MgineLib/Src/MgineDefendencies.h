#pragma once

#pragma region Defendencies
# pragma comment (lib, "d3d9.lib")
# pragma comment (lib, "dinput8.lib")
# pragma comment (lib, "dxguid.lib")
# pragma comment (lib, "dsound.lib")
# pragma comment (lib, "dxerr.lib")
# pragma comment (lib, "winmm.lib")
# if _DEBUG
#  pragma comment (lib, "d3dx9d.lib")
#  pragma comment (lib, "libpng15d.lib")
#  pragma comment (lib, "zlibstatd.lib")
#  pragma comment (lib, "MgineLibd.lib")
#  pragma comment (lib, "libsndfile_1_0_25_portd.lib")
# else
#  pragma comment (lib, "d3dx9.lib")
#  pragma comment (lib, "libpng15.lib")
#  pragma comment (lib, "zlibstat.lib")
#  pragma comment (lib, "MgineLib.lib")
#  pragma comment (lib, "libsndfile_1_0_25_port.lib")
# endif

// dx / sound related
# ifdef _DEBUG
#  define D3D_DEBUG_INFO
# endif
# if defined(DIRECTINPUT_VERSION) && DIRECTINPUT_VERSION != 0x800
#  error "DirectInput Version is not matched"
# endif
# define DIRECTINPUT_VERSION 0x800
# include <d3d9.h>
# include <d3dx9.h>
# include <dinput.h>
# include <MMSystem.h>
# include <dsound.h>
# include <DxErr.h>

// stl
# include <assert.h>
# include <new>
# include <utility>

// boost
# include <boost/signal.hpp>
# include <boost/bind.hpp>
# include <boost/ptr_container/ptr_vector.hpp>
# include <boost/ptr_container/ptr_list.hpp>
#pragma endregion