# flying_fish

Welcome.  This is the repository containing code and OSH files for the solar sounder known as Flying Fish. It's a solar-powered, temperature and light controlled board. you can buy the circuit board, soon, at <a href="https://monadmonad.square.site/">my ghost company, Monad</a>. It also will work on vanilla Arduino UNO but you'll need to provide peripherals such as temp sensor, amp, etc. 

here's the image of the pcb:
<img src= "https://raw.githubusercontent.com/dfict/flying_fish/main/FlyingFish_pcb.jpg" alt="the picture of the pcb!">

here's the schematic for the board, in case you need to debug:

<img src="https://raw.githubusercontent.com/dfict/flying_fish/main/FlyingFish_schematic.png" alt="the schematic is here">

<a href="https://www.instagram.com/stories/highlights/18321952708145409/">Here are two Flying Fish singing!</a>

I'm working on testing the BOM.
<a href="https://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=30a4665741">Here's the project BOM from Mouser.</a>

A few notes that will be incorporated in the next revision:
1) the PCB itself calls for an LM7805, but I prefer using a low drop out regulator, LF50ABV for example, so I've included that in the BOM. The pinout is the same, but make sure you use the right capacitors for that one—.1uf on input, 2.2uf on output. Different from the board text!
2) the NJM2073 is obsolete. You can use the TDA2822D instead, but that is surface mount, so you can use a surfboard for that, or just scoop the 2073 on ebay. Some stores in Akihabara, Japan still carry the NJM2073 for 30 yen a piece!
3) for programming the chip, you can use an ISP programmer, or even use an Arduino as ISP. I'm going to update this later with a guide for that.
4) the potentiometer silkscreen is on the bottom of the board. a mistake! you can short out two of the pins if you don't want to use a volume knob. (the two next to the 10k resistor on the topside). Or you can use a panel mount pot. or use one of the two included on the bom, topside.
5) for the solar Panel: 9v, 300ma works great. You can scoop them anywhere.
6) box design files coming soon!