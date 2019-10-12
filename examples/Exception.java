public class Exception {
	
	public static void main(String[] args) {
        int num1, num2;
        try {
        	throw new ArithmeticException("/ by zero");
        } catch (ArithmeticException e) {
            System.out.print(e);
        }
	}
}
