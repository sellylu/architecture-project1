for var in ~/Archi2016_Project1/archiTA/testcase/open_testcase/* ~/Archi2016_Project1/archiTA/testcase/hidden_testcase/* ~/Archi2016_Project1/archiTA/testcase/Student_valid_testcase/*
do
	cp ${var}/iimage.bin ./
	cp ${var}/dimage.bin ./
	~/Archi2016_Project1/102061152_01/simulator/single_cycle
	diff snapshot.rpt ${var}/snapshot.rpt
	diff error_dump.rpt ${var}/error_dump.rpt
	echo Test ${var} end
done

rm *.bin *.rpt
