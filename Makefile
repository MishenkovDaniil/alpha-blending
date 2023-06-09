CC = g++
Flags = -o3 -mavx2 -lsfml-graphics -lsfml-window -lsfml-system -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

naive:  implementation_versions/alpha_blending_0.cpp root_mean_square/rms.cpp
		$(CC) $(Flags) implementation_versions/alpha_blending_0.cpp root_mean_square/rms.cpp -o not-optimized
		./not-optimized
not_optimized: implementation_versions/alpha_blending_1.cpp root_mean_square/rms.cpp	
		$(CC) $(Flags) implementation_versions/alpha_blending_1.cpp root_mean_square/rms.cpp -o optimized-1
		./optimized-1
optimized: implementation_versions/alpha_blending_2.cpp	 root_mean_square/rms.cpp
		$(CC) $(Flags) implementation_versions/alpha_blending_2.cpp root_mean_square/rms.cpp -o optimized-2
		./optimized-2
