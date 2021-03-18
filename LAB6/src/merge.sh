# chmod 755 merge.sh

rm -rf temp
mkdir temp
rm *.img

nasm boot.asm -o ./temp/boot.bin
nasm PTB.asm -o ./temp/PTB.bin

cd userprog
nasm tl.asm -o ../temp/tl.bin
nasm tr.asm -o ../temp/tr.bin
nasm bl.asm -o ../temp/bl.bin
nasm br.asm -o ../temp/br.bin
nasm syscall.asm -o ../temp/syscall.bin
cd ..

nasm -f elf32 save.asm -o ./temp/save.o

nasm -f elf32 monitor.asm -o ./temp/monitor.o
nasm -f elf32 lib_asm.asm -o ./temp/lib_asm.o
gcc -c -m16 -march=i386 -masm=intel -nostdlib -ffreestanding -mpreferred-stack-boundary=2 -lgcc -shared lib_c.c -o ./temp/lib_c.o -fno-pie
ld -m elf_i386 -N -Ttext 0x8000 --oformat binary ./temp/monitor.o ./temp/lib_asm.o ./temp/lib_c.o ./temp/save.o -o ./temp/lib_c.bin
rm ./temp/*.o

dd if=./temp/boot.bin of=boot.img bs=512 count=1 2> /dev/null
dd if=./temp/PTB.bin of=boot.img bs=512 seek=1 count=1 2> /dev/null
dd if=./temp/lib_c.bin of=boot.img bs=512 seek=2 count=16 2> /dev/null
dd if=./temp/tl.bin of=boot.img bs=512 seek=18 count=2 2> /dev/null
dd if=./temp/tr.bin of=boot.img bs=512 seek=20 count=2 2> /dev/null
dd if=./temp/bl.bin of=boot.img bs=512 seek=22 count=2 2> /dev/null
dd if=./temp/br.bin of=boot.img bs=512 seek=24 count=2 2> /dev/null
dd if=./temp/syscall.bin of=boot.img bs=512 seek=26 count=2 2> /dev/null
rm *.bin

echo "Done"