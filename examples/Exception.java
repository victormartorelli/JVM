public class Exception {

	public static void main(String[] args) {
        int num1, num2;
        try {
            num1 = 0;
            num2 = 62 / num1;
        } catch (ArithmeticException e) {
            System.out.print(e);
        }
	}

}
