#pragma once

#undef VIVIUM_ASSERT
#undef VIVIUM_VK_CHECK

#if !VIVIUM_NO_VECTOR_DEFINES
#undef I32x2
#undef U32x2
#undef F32x2
#undef F64x2
#endif

#undef VIVIUM_NO_VECTOR_DEFINES
#undef VIVIUM_PHYSICS_MAX_GROUPS

// Some things not undefined since user may use them
// #undef VIVIUM_NULL_HANDLE