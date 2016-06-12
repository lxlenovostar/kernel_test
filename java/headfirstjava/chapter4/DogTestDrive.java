class DogTestDrive {
    
    public static void main (String[] args) {
        Dog one = new Dog();
        Dog two = new Dog();

        String a = new String("abc");
        String b = new String("abc");

        if (a.equals(b))
            System.out.println("true"); 
        else 
            System.out.println("false"); 

        if (one.equals(two))
            System.out.println("true"); 
        else 
            System.out.println("false"); 
    }
}
