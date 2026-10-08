#pragma once

// Compatibility definitions removed from modern DirectX headers.
#ifndef D3DVAL
#define D3DVAL(value) static_cast<float>(value)
#endif
