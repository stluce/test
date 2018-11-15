I include just part of this work because it maybe need to change license

BriFT - The text preprocessor Bricks + Shift
Allow to define named a small pieces of text(Bricks) which can be easy inserted to any text
Brift is text preprocessor. I use it to create the static html pages. It allow to define and reuse some part of texts. One named parts of text I called 'BRICK'. You can use define some BRICKs in brick definition file .brk, next you can easy make page from these bricks.

Default.brk - default brick definition file
BRICK definition
BRICKNAME=Text to be placed in results, it can include BriFT macros # commentary

MULTILINEBRICK=[


]

Functions:

$FUNCTION_NAME(Parameters~))

$TIME - insert current(ie processing) time
$DATE - insert current date
$BRICKS $WALL  - include custom brick definition file $BRICKS(filename) $WALL(path/to/file)
$BRICK - define custom brick $BRICK(SOME=sometext)

Source flow control command its like C prerocessor

$INCLUDE  - include source file $INCLUDE(path\to\file)

// like C prerocessor #ifdef #elif #else #endif 
$IFDEF  - if brick is defined   $IFDEF(_DEBUG_)

$IFNDEF - if brick is not defined $IFNDEF(_USE_CUSTOM_TAG_)

$ELIF	- else if brick is defined $ELIF(USE_IE_9)

$ELIFN	- else if brick is defined $elseifn(_DEBUG_)

$ELSE	- else ie $ifDEF (_DEBUF_) this is debug version $elseif current verions is $VERSION $elsif

$ENDIF  - end of if

.
