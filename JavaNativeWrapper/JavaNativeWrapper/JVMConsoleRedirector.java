import java.io.*;

public class JVMConsoleRedirector {
    public static void redirect(String path) {
        try {
            PrintStream ps = new PrintStream(new FileOutputStream(path, true), true);
            System.setOut(ps);
            System.setErr(ps);
            System.out.println("[Java] System.out redirected successfully.");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}