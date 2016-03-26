for var in branch func recur fib_dp error2
do
	cp ~/Archi2016_Project1/archiTA/testcase/open_testcase/${var}/iimage.bin ./
	cp ~/Archi2016_Project1/archiTA/testcase/open_testcase/${var}/dimage.bin ./
	~/Archi2016_Project1/102061152_01/simulator/single_cycle
	diff snapshot.rpt ~/Archi2016_Project1/archiTA/testcase/open_testcase/${var}/snapshot.rpt
	diff error_dump.rpt ~/Archi2016_Project1/archiTA/testcase/open_testcase/${var}/error_dump.rpt
done

cp ~/Archi2016_Project1/102061152_01/testcase/iimage.bin ./
cp ~/Archi2016_Project1/102061152_01/testcase/dimage.bin ./
~/Archi2016_Project1/102061152_01/simulator/single_cycle
diff snapshot.rpt ~/Archi2016_Project1/102061152_01/testcase/snapshot.rpt
diff error_dump.rpt ~/Archi2016_Project1/102061152_01/testcase/error_dump.rpt

rm *.bin *.rpt
