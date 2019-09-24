public class Catch {
    public static void main(String[] args) {
        try {
            int b = 0;
            int a = 3 / b;
        } catch(Exception e){
            e.printStackTrace();
        }
    }
}
