class Dog {
    int size;
    String name; 

    void bark() {
        if (size > 60) {
            System.out.println("Woof");
        } else if (size > 14) {
            System.out.println("Ruff");
        } else {
            System.out.println("Yip");
        }
    }
}
