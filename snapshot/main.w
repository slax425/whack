module Main

extern func puts(char*) int;

func t2() bool {
	return true;
}

type Ra interface {
	func(() -> char*) Jah;
}

type Ta interface {
	func(() -> char*) Duh;
	func(() -> char*) Vuh;
}

type Gool interface : Ra, Ta {
	func(() -> bool) Cas;
	func(() -> int) Num;
}

func Test(Gool b) bool {
	return b.Cas();
}

func TestB(Ra b) char* {
	_ = puts(b.Jah());
	return b.Jah();
}

type Rest struct { char* msg; }

func (Rest) amazing(bool rr) int @inline {
	_ = t2();
	if rr puts(this.msg);
	return 0;
}

func (Rest) Jah() char* {
	return "Rest";
}

func ttt() {
	Rest r;
	_ = TestB(r);
}

func (Rest) ama(int b) int @inline {
	t2();
	return b;
}

func testPartialApply(char* one, char* two) {
	_ = puts(one);
	_ = puts(two);
}

func closureExample1(bool p) auto {
	return func(bool a) auto {
		return func(int x) func(() -> bool) @mustinline {
			return func() bool {
				return x < 10;
			};
		};
	};
}

func testClosureExample1() {
	if closureExample1(true)(true)(0)() {
		_ = puts("closureExample1");
	}
}

func closureExample2(int a, int b) auto {
	return func [a, b, c = a + b]() auto {
		return c == a + b;
	};
}

func testClosureExample2() {
	if closureExample2(2, 1)() {
		_ = puts("closureExample2: pass");
	}
}

func testDynamicMemoryAndClosures() {
	let mem = new Rest;
	defer delete mem;
	let r = *mem;
	r.msg = "Hello, World!";
	t2->r.amazing();
	testClosureExample1();
	testClosureExample2();
}

func main(int argc, char** argv) int {
	Rest r;
	r.msg = "Morty C137";
	let ret = r.amazing->r.ama(true);
	match type(r.amazing) {
	  func(bool -> int): puts("Rest#amazing");
	}
	let apply = testPartialApply("Foo, ", ...);
	apply("Bar!");
	testDynamicMemoryAndClosures();
	ttt();
	return ret;
}
