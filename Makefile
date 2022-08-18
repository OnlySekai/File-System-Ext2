SRC = $(wildcard *.c)
OBJ = ${SRC:.c=.o}
HEA = $(wildcard *.h)

all: checkdisk imp exp tch stt cpf cpd tree mov format mkd mkf rmf rmd myls

checkdisk: check_alloc.o cache.o util.o alloc.o
	gcc -o checkdisk check_alloc.o cache.o util.o alloc.o 

imp: import.o cache.o util.o alloc.o 
	gcc -o imp import.o cache.o util.o alloc.o

exp: export.o cache.o util.o alloc.o
	gcc -o exp export.o cache.o util.o alloc.o

tch: touch.o cache.o util.o alloc.o
	gcc -o tch touch.o cache.o util.o alloc.o

stt: stat.o cache.o util.o alloc.o
	gcc -o stt stat.o cache.o util.o alloc.o

cpf: copy_file.o util.o cache.o alloc.o
	gcc -o cpf copy_file.o util.o cache.o alloc.o
 
cpd: copy_dir.o util.o cache.o alloc.o
	gcc -o cpd copy_dir.o util.o cache.o alloc.o
 
tree: tree.o cache.o util.o alloc.o
	gcc -o tree tree.o cache.o util.o alloc.o 
 
mov: move.o cache.o util.o alloc.o
	gcc -o mov move.o cache.o util.o alloc.o 
 
format: format.o cache.o util.o alloc.o
	gcc -o format util.o format.o cache.o alloc.o -lm

mkd: mkd_final.o cache.o util.o libtuan.o alloc.o
	gcc -o mkd mkd_final.o cache.o util.o libtuan.o alloc.o 

mkf: mkf_final.o cache.o util.o libtuan.o alloc.o
	gcc -o mkf mkf_final.o cache.o util.o libtuan.o alloc.o 

rmf: rmf_final.o cache.o util.o libtuan.o alloc.o
	gcc -o rmf rmf_final.o cache.o util.o libtuan.o alloc.o 

rmd: rmd_final.o cache.o util.o libtuan.o alloc.o
	gcc -o rmd rmd_final.o cache.o util.o libtuan.o alloc.o 
 
myls: ls.o cache.o util.o alloc.o libtuan.o
	gcc -o myls ls.o cache.o util.o alloc.o libtuan.o 
                                                              
%.c: $(HEA)

%.o: %.c
	gcc -D_GNU_SOURCE -c -Wall -O2 -o $@ $<

clean:
	rm *.o    
	rm checkdisk imp exp tch stt cpf cpd tree mov format mkd mkf rmf rmd myls
