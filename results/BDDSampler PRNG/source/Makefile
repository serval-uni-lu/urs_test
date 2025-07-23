CFLAGS = -Iinclude 
LDFLAGS = -static -Llib -ltestu01 -lprobdist -lmylib -lm

all: bc_r31 bc_r32 bc_r32v2 bc_rd c_r31 c_r32 c_r32v2 c_rd sc_r31 sc_r32 sc_r32v2 sc_rd

bc_r31   : src/bc_r31.c  
	gcc -o bc_r31   src/bc_r31.c   $(CFLAGS) $(LDFLAGS)
bc_r32   : src/bc_r32.c  
	gcc -o bc_r32   src/bc_r32.c   $(CFLAGS) $(LDFLAGS)
bc_r32v2 : src/bc_r32v2.c
	gcc -o bc_r32v2 src/bc_r32v2.c $(CFLAGS) $(LDFLAGS)
bc_rd    : src/bc_rd.c   
	gcc -o bc_rd    src/bc_rd.c    $(CFLAGS) $(LDFLAGS)
c_r31    : src/c_r31.c   
	gcc -o c_r31    src/c_r31.c    $(CFLAGS) $(LDFLAGS)
c_r32    : src/c_r32.c   
	gcc -o c_r32    src/c_r32.c    $(CFLAGS) $(LDFLAGS)
c_r32v2  : src/c_r32v2.c 
	gcc -o c_r32v2  src/c_r32v2.c  $(CFLAGS) $(LDFLAGS)
c_rd     : src/c_rd.c    
	gcc -o c_rd     src/c_rd.c     $(CFLAGS) $(LDFLAGS)
sc_r31   : src/sc_r31.c  
	gcc -o sc_r31   src/sc_r31.c   $(CFLAGS) $(LDFLAGS)
sc_r32   : src/sc_r32.c  
	gcc -o sc_r32   src/sc_r32.c   $(CFLAGS) $(LDFLAGS)
sc_r32v2 : src/sc_r32v2.c
	gcc -o sc_r32v2 src/sc_r32v2.c $(CFLAGS) $(LDFLAGS)
sc_rd    : src/sc_rd.c   
	gcc -o sc_rd    src/sc_rd.c    $(CFLAGS) $(LDFLAGS)

package: all
	tar cf test.tar bc_r31 bc_r32 bc_r32v2 bc_rd c_r31 c_r32 c_r32v2 c_rd sc_r31 sc_r32 sc_r32v2 sc_rd

clean:
	rm bc_r31 bc_r32 bc_r32v2 bc_rd c_r31 c_r32 c_r32v2 c_rd sc_r31 sc_r32 sc_r32v2 sc_rd

