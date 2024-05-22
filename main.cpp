#include <cstdlib>
#include <fstream>
#include <iostream>
#include<string>
#include <ctime>

const int ALLOC_SIZE = 10, MAX_OPTIONS = 20;
const int NIL = -1;

// Program state
const int EXIT = NIL, START = 0, LOGIN = 1, SIGNUP = 2, LOBBY = 3, SHOW_PRODUCTS = 4, ADD_TO_CART = 5, BUY_PRODUCT = 6, RETURN_PRODUCT = 7, REVIEW_PRODUCT = 8, ADD_PRODUCT = 9, LOG_OUT = 10;

// Product status pseudo enum
const int SENT = 0, DELIVERED = 1, RETURNED = 2;
//Product pseudo enum
const int ID = 0;
// User pseudo enum
const int NAME = 0, AGE = 1, EMAIL = 2, PASSWORD = 3, WISHLIST = 4;
const int PAYMENT_METHOD = 5, CART = 6, HISTORY = 7;

struct User
{
    int id;
    std::string name = "John Doe";
    int age = 33;
    std::string email = "johndoe@gmail.com";
    std::string password;
    int wishlist[ALLOC_SIZE] = {NIL};
    int payment_method = NIL;
    int cart[ALLOC_SIZE] = {NIL};
    int cart_len = 0;
    int history[ALLOC_SIZE] = {NIL};
    int history_len = 0;
    int is_null = false;
};

struct Dimensions
{
    float x;
    float y;
    float z;
};

struct Product
{
    int id = NIL;
    std::string name;
    float price = NIL;
    Dimensions dimensions = {NIL, NIL, NIL};
    float weight = NIL;
    float stars = NIL;
    std::string comments[ALLOC_SIZE] = {"no comments"};
    std::string vendor = {"John Doe"};
    bool is_null = false;
};

struct User_Table
{
    std::string path = "user_table.txt";
    std::string headers = "id,name,age,email,password,wishlist,payment_method,cart,history,";
    User users[ALLOC_SIZE];
    int len = 0;
};

struct Product_Table
{
    std::string path = "product_table.txt";
    std::string headers = "id,name,price,x,y,z,weight,comments,stars,vendor,";
    Product products[ALLOC_SIZE];
    int len = 0;
};

struct Tables
{
    std::string path;
    User_Table user_table;
    Product_Table product_table;
    bool is_null = false;
    int current_user;
};


Tables init_tables(Tables t); //*
int run(int program_state, Tables t[]);
int start(); // *
int login(Tables t[]); //*
int signup(Tables t[]); //*
int lobby(); //*
int show_products(Product_Table p); //*
int add_product(Tables t[]); //*
int logout(Tables t[]); //*
int buy(Tables t[]); //*
int exit_program(Tables t[]); //*
int return_product(Tables t[]);
int add_to_cart(Tables t[]); //*

int read_user_table(User users[]); //*
void write_user_table(User_Table user_table); //*
int read_product_table(Product products[]); //*
void write_product_table(Product_Table product_table); //*
User find_user_table(int user_header, std::string query, Tables t);
Product find_product_table(std::string query, Tables t); //*
int product_status(int id, int status); //*
void print_products(Product_Table p); //*
void generate_receipt(Product p[], std::string client);
void generate_return_receipt(Product p[], std::string sender, std::string receiver); //*
int break_composed_id(int composed_id);

char ask(std::string question); //*
bool file_exists(std::string path); //*
int tokenizer_csv(std::string line, std::string dest[]); //*
int tokenizer_dot(std::string line, std::string dest[]); //*
int tokenizer_dot_int(std::string line, int dest[]);
int touch(std::string path, std::string headers); //*
void stoia(std::string line, int dest[]); //*
std::string satos(std::string arr[]); //*
std::string iatos(int arr[]); //*
void memoset(int arr[], int len, int val); //*
void copy_user_array(User source[], User destination[], int size); //*
void copy_product_array(Product source[], Product destination[], int size); //*
Tables set_default_values(Tables t); //*
//

int main()
{
    Tables t;
    int program_state = START;

    bool is_running = true;
    t = init_tables(t);

    if (t.is_null)
    {
        std::cout << "Error initializing tables" << std::endl;
        return 1;
    }

    Tables ptable[] = {t};

    while(is_running)
    {
        program_state = run(program_state, ptable);

        if (program_state == NIL)
            is_running = false;
    }
}

Tables init_tables(Tables t)
{
    int err = 0;

    User user_buff[ALLOC_SIZE];
    User_Table user_table;

    for (int i = 0 ; i < ALLOC_SIZE ; ++i)
    {
        memoset(user_table.users[i].wishlist, ALLOC_SIZE, NIL);
        memoset(user_table.users[i].cart, ALLOC_SIZE, NIL);
        memoset(user_table.users[i].history, ALLOC_SIZE, NIL);
    }

    Product product_buff[ALLOC_SIZE];
    Product_Table product_table;

    if (!file_exists(t.user_table.path))
    {

        err = touch(t.user_table.path, t.user_table.headers);

        if (err == NIL)
        {
            Tables null_t;
            null_t.is_null = true;
            std::cout << "Error creating file" << std::endl;
            return null_t;
        }
    }

    user_table.len = read_user_table(user_buff);
    copy_user_array(user_buff, user_table.users, user_table.len);


    if(!file_exists(t.product_table.path))
    {
        err = touch(t.product_table.path, t.product_table.headers);

        if (err == NIL)
        {
            Tables null_t;
            null_t.is_null = true;
            std::cout << "Error creating file" << std::endl;
            return null_t;
        }
    }

    product_table.len = read_product_table(product_buff);
    copy_product_array(product_buff, product_table.products, product_table.len);

    t.user_table = user_table;
    t.product_table = product_table;

    if (t.user_table.len == 0 && t.product_table.len == 0)
    {
        char opt = ask("There's no data, would you like to generate default data Y/N?");
        if (opt == 'Y' || opt == 'y')
        {
            t = set_default_values(t);
            write_user_table(t.user_table);
            write_product_table(t.product_table);
        }
    }

    return t;
}

int read_user_table(User users[])
{
    std::string s;
    int position = 0;
    int lines_read = 0;

    std::ifstream f("user_table.txt");
    if (f.is_open())
    {
        std::getline(f, s);

        while(std::getline(f, s))
        {
            ++lines_read;
            std::string temp[ALLOC_SIZE] = {""};
            User u;

            memoset(u.wishlist, ALLOC_SIZE, NIL);
            memoset(u.cart, ALLOC_SIZE, NIL);
            memoset(u.history, ALLOC_SIZE, NIL);

            tokenizer_csv(s, temp);
            u.id = std::stoi(temp[0]);
            u.name = temp[1];
            u.age = std::stoi(temp[2]);
            u.email = temp[3];
            u.password = temp[4];
            tokenizer_dot_int(temp[5], u.wishlist);
            u.payment_method = std::stoi(temp[6]);
            u.cart_len = tokenizer_dot_int(temp[7], u.cart);
            u.history_len = tokenizer_dot_int(temp[8], u.history);

            users[position] = u;
            ++position;
        }
        f.close();
    }
    else
        std::cout << "Error opening file" << std::endl;

    return lines_read;
}

int read_product_table(Product products[])
{
    std::string s;
    int position = 0;
    int lines_read = 0;

    std::ifstream f("product_table.txt");
    if (f.is_open())
    {
        std::getline(f, s);

        while(std::getline(f, s))
        {
            ++lines_read;
            std::string temp[ALLOC_SIZE] = {""};
            Product p;

            tokenizer_csv(s, temp);
            p.id = std::stoi(temp[0]);
            p.name = temp[1];
            p.price = std::stof(temp[2]);
            p.dimensions.x = std::stof(temp[3]);
            p.dimensions.y = std::stof(temp[4]);
            p.dimensions.z = std::stof(temp[5]);
            p.weight = std::stoi(temp[6]);
            tokenizer_dot(temp[7], p.comments);
            p.stars = std::stof(temp[8]);
            p.vendor = temp[9];

            products[position] = p;
            ++position;
        }
        f.close();
    }
    else
        std::cout << "Error opening file" << std::endl;

    return lines_read;
}

void write_user_table(User_Table user_table)
{
    std::ofstream f("user_table.txt");

    if (f.is_open())
    {

        f << user_table.headers << std::endl;

        for (int i = 0 ; i < user_table.len ; ++i)
        {
            f <<
              user_table.users[i].id
              <<
              ","
              <<
              user_table.users[i].name
              << ","
              <<  user_table.users[i].age
              << ","
              << user_table.users[i].email
              << ","
              << user_table.users[i].password
              << ","
              <<  iatos(user_table.users[i].wishlist)
              << ","
              << user_table.users[i].payment_method
              << ","
              <<  iatos(user_table.users[i].cart)
              << ","
              << iatos(user_table.users[i].history)
              << ","
              << std::endl;
        }
        std::cout << "Saving..." << std::endl;
    }
    else
        std::cout << "Error opening file" << std::endl;
}

void write_product_table(Product_Table product_table)
{
    std::ofstream f("product_table.txt");

    if (f.is_open())
    {

        f << product_table.headers << std::endl;

        for (int i = 0 ; i < product_table.len ; ++i)
        {

            f <<
              product_table.products[i].id
              <<
              ","
              <<
              product_table.products[i].name
              << ","
              <<  product_table.products[i].price
              << ","
              << product_table.products[i].dimensions.x
              << ","
              << product_table.products[i].dimensions.y
              << ","
              << product_table.products[i].dimensions.z
              << ","
              << product_table.products[i].weight
              << ","
              <<  satos(product_table.products[i].comments)
              << ","
              << product_table.products[i].stars
              << ","
              << product_table.products[i].vendor
              << ","
              << std::endl;
        }
        std::cout << "Saving..." << std::endl;
    }
    else
        std::cout << "Error opening file" << std::endl;
}


int run(int program_state, Tables t[])
{

    switch (program_state) {
        case EXIT:
        {
            return exit_program(t);
        }
        case START:
        {
            return start();
        }
            break;
        case LOGIN:
        {
            return login(t);
        }
            break;
        case SIGNUP:
        {
            return signup(t);
        }
            break;
        case LOBBY:
        {
            return lobby();
        }
            break;
        case SHOW_PRODUCTS:
        {
            return show_products(t[0].product_table);
        }
            break;
        case ADD_TO_CART:
        {
            return add_to_cart(t);
        }
            break;
        case BUY_PRODUCT:
        {
            return buy(t);
        }
            break;
        case RETURN_PRODUCT:
        {
            return return_product(t);
        }
            break;
        case REVIEW_PRODUCT:
            break;
        case ADD_PRODUCT:
        {
            return add_product(t);
        }
            break;
        case LOG_OUT:
        {
            return logout(t);
        }
            break;
        default:
            return 0;
    }

    return program_state;
}

int start()
{
    int opt = 0;

    std::cout << "MENU" << std::endl;
    std::cout << "1. Log in" << std::endl;
    std::cout << "2. Sign up" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "-> ";
    std::cin >> opt;

    switch (opt)
    {
        case 0:
            return start();
        case 1:
            return LOGIN;
        case 2:
            return SIGNUP;
        case 3:
            return EXIT;
        default:
        {
            std::cout << "Invalid option, try again" << std::endl;
            return start();
        }
    }
}

int signup(Tables t[])
{
    User user;
    std::string name;
    int age;
    std::string email;
    std::string password;
    std::string temp;

    std::cout << "Sign up" << std::endl;

    std::cout << "Name: ";
    std::cin >> name;

    std::cout << "Age: ";
    std::cin >> age;

    if (age < 18)
    {
        std::cout << "You must be over 18 to sign up, bye..." << std::endl;
        return EXIT;
    }

    std::cout << "Email: ";
    std::cin >> email;

    std::cout << "Password: ";
    std::cin >> password;

    std::cout << "Confirm password: ";
    std::cin >> temp;

    if (temp != password)
    {
        std::cout << "You might made a typo, try again" << std::endl;
        return signup(t);
    }

    t->user_table.users[t->user_table.len].email = email;
    t->user_table.users[t->user_table.len].password = password;
    t->user_table.users[t->user_table.len].name = name;
    t->user_table.users[t->user_table.len].age = age;
    t->user_table.users[t->user_table.len].id = t->user_table.len + 1;

    ++t->user_table.len;

    write_user_table(t->user_table);

    return START;
}


int login(Tables t[])
{
    User user;
    std::string email;
    std::string password;
    bool error_email = false;
    bool error_password = false;
    int status = NIL;

    std::cout << "LOGIN" << std::endl;
    std::cout << "Email: ";
    std::cin >> email;

    std::cout << "Password: ";
    std::cin >> password;


    user = find_user_table(EMAIL, email, t[0]);
    error_email= user.is_null;

    if (user.password == password)
        error_password = false;
    else
        error_password = true;

    if (!error_email && !error_password)
    {
        t[0].current_user = user.id;
        return status = LOBBY;
    }

    std::cout << "Invalid credentials" << std::endl;
    return START;
}

int logout(Tables t[])
{
    write_user_table(t[0].user_table);
    write_product_table(t[0].product_table);
    t->current_user = 0;
    std::cout << "Loggin out..." << std::endl;
    return EXIT;
}

int lobby()
{
    int opt = 0;

    std::cout << "MENU" << std::endl;
    std::cout << "1. Show products" << std::endl;
    std::cout << "2. Add to the cart" << std::endl;
    std::cout << "3. Buy product" << std::endl;
    std::cout << "4. Return product" << std::endl;
    std::cout << "5. Review product" << std::endl;
    std::cout << "6. Add product" << std::endl;
    std::cout << "7. Log out" << std::endl;
    std::cout << "-> ";
    std::cin >> opt;

    switch (opt)
    {
        case 0:
            return start();
        case 1:
            return SHOW_PRODUCTS;
        case 2:
            return ADD_TO_CART;
        case 3:
            return BUY_PRODUCT;
        case 4:
            return RETURN_PRODUCT;
        case 5:
            return REVIEW_PRODUCT;
        case 6:
            return ADD_PRODUCT;
        case 7:
            return LOG_OUT;
        default:
        {
            std::cout << "Invalid option, try again" << std::endl;
            return start();
        }
    }
}


User find_user_table(int user_header, std::string query, Tables t)
{
    User null_user;
    null_user.is_null = true;

    switch (user_header) {
        case NAME:
        {
            for (int i = 0 ; i < t.user_table.len ; ++i )
            {
                if (t.user_table.users[i].name == query)
                    return t.user_table.users[i];
            }
            return null_user;
        }
        case AGE:
        {
            printf("Processing age...\n");
            for (int i = 0 ; i < t.user_table.len ; ++i )
            {
                if (std::to_string(t.user_table.users[i].age) == query)
                    return t.user_table.users[i];
            }
            return null_user;
        }
        case EMAIL:
        {
            for (int i = 0 ; i < t.user_table.len ; ++i )
            {
                if (t.user_table.users[i].email == query)
                    return t.user_table.users[i];
            }
            return null_user;
        }
        case PASSWORD:
        {
            for (int i = 0 ; i < t.user_table.len ; ++i )
            {
                if (t.user_table.users[i].password == query)
                    return t.user_table.users[i];
            }
            return null_user;
        }
        default:
            return null_user;
            break;
    }

    return null_user;
}

Product find_product_table(std::string query, Tables t)
{
    Product null_product;
    null_product.is_null = true;

    for (int i = 0 ; i < t.product_table.len ; ++i )
    {
        if (t.product_table.products[i].id == std::stoi(query))
            return t.product_table.products[i];
    }
    return null_product;
}

int add_product(Tables t[])
{
    Product p;
    char opt;

    std::cout << "ADD PRODUCTS" << std::endl;

    p.id = t[0].product_table.len + 1;
    p.vendor = t[0].user_table.users[t[0].current_user - 1].name;

    std::cout << "Name: ";
    std::cin >> p.name;

    std::cout << "Price: ";
    std::cin >> p.price;

    std::cout << "Height: ";
    std::cin >> p.dimensions.y;

    std::cout << "Width: ";
    std::cin >> p.dimensions.x;

    std::cout << "Depth: ";
    std::cin >> p.dimensions.z;

    std::cout << "Weight: ";
    std::cin >> p.weight;


    while(true)
    {
        opt = ask("Do you want to overwrite any property? Y/N" );
        if (opt == 'Y' || opt == 'y')
            return add_product(t);
        else if (opt == 'N' || opt == 'n')
            break;
    }

    while(true)
    {
        opt = ask("Would you like to save the product? Y/N" );
        if (opt == 'Y' || opt == 'y')
        {
            t[0].product_table.products[t[0].product_table.len] = p;
            ++t[0].product_table.len;
            write_product_table(t[0].product_table);
            break;
        } else if (opt == 'N' || opt == 'n')
            break;
    }
    return LOBBY;
}

int show_products(Product_Table p)
{
    std::cout << "PRODUCTS" << std::endl;
    print_products(p);

    std::cout << std::endl;
    while (true)
    {
        char opt = ask("Press c to continue");
        if (opt == 'c' || opt == 'C')
            return LOBBY;
    }
}

int review_product(Tables t[])
{
    int opt;
    int history_len = t[0].user_table.users[t[0].current_user - 1].history_len;
    int id;
    Product_Table p;
    Product rp;

    std::cout << "REVIEW PRODUCT" << std::endl;

    std::cout << "Type the product's id you'd like to review" << std::endl;

    for (int i = 0 ; i < history_len; ++i)
    {
        id = break_composed_id(t[0].user_table.users[t[0].current_user - 1].history[i]);
        p.products[i] = find_product_table(std::to_string(id), t[0]);
        ++p.len;
    }
    print_products(p);

    std::cout << "-> ";
    std::cin >> opt;

    return LOBBY;
}

void print_products(Product_Table p)
{
    for (int i = 0 ; i < p.len ; ++i)
    {
        if (i % 2 != 0)
        {
            std::cout << "ID: " << p.products[i].id << "\t Name: " << p.products[i].name << "\t Price: " << p.products[i].price << "\t | " << std::endl;
        }
        else

            std::cout << "ID: " << p.products[i].id << "\t Name: " << p.products[i].name << "\t Price: " << p.products[i].price << "\t | \t";
    }
}

int add_to_cart(Tables t[])
{
    std::string id;
    char opt;
    Product res;
    int position = 0;

    // Set position according to the cart length
    if (t[0].user_table.users[t[0].current_user].cart_len != 0)
        position = t[0].user_table.users[t[0].current_user].cart_len - 1;

    std::cout << "ADD TO CART" << std::endl;

    while(true)
    {
        std::cout << "Type the product's ID" << std::endl;
        std::cout << "-> ";
        std::cin >> id;
        res = find_product_table(id, t[0]);
        if (res.is_null)
        {
            std::cout << "The ID doesn't exist" << std::endl;
            opt = ask("Try again or go back? A/B");
            if (opt == 'a' || opt == 'A')
                continue;
            else
                break;
        }
        else
        {
            std::cout << "Found product: " << res.name << std::endl;
            opt = ask("Confirm selection? Y/N");
            if (opt == 'y' || opt == 'Y')
            {
                t[0].user_table.users[t[0].current_user - 1].cart[position] = res.id;
                ++position;
                ++t[0].user_table.users[t[0].current_user - 1].cart_len;

                std::cout << "Product added..." << std::endl;
                opt = ask("Continue adding? Y/N");
                if (opt == 'y' || opt == 'Y')
                    continue;
                else
                    break;
            }
            else
                opt = ask("Go back? Y/N");
            if (opt == 'a' || opt == 'A')
                continue;
            else
                break;
        }
    }
    return LOBBY;
}

int buy(Tables t[])
{
    char opt;
    int user_index = t[0].current_user - 1;
    std::string query;
    Product res;
    int position_history = 0;
    Product bought[ALLOC_SIZE];
    int position = 0;

    // Set position according to the cart and history length
    if (t[0].user_table.users[t[0].current_user].history_len != 0)
        position_history = t[0].user_table.users[t[0].current_user].history_len - 1;

    if (t[0].user_table.users[t[0].current_user].cart_len != 0)
        position_history = t[0].user_table.users[t[0].current_user].cart_len - 1;

    std::cout << "CHECKOUT" << std::endl;

    if (t[0].user_table.users[user_index].cart[0] == NIL)
    {
        std::cout << "You haven't added products to the cart" << std::endl;
        return LOBBY;
    }

    opt = ask("Start buying Y/N");
    if (opt == 'Y' || opt == 'y')
    {
        int new_cart_len = t[0].user_table.users[user_index].cart_len;
        int index = 0;
        while (t[0].user_table.users[user_index].cart[index] != NIL)
        {
            query = std::to_string(t[0].user_table.users[user_index].cart[0]);
            res = find_product_table(query, t[0]);
            std::string statement = "Buy " + res.name + " for $" + std::to_string(res.price) + " Y/N?";
            opt = ask(statement);
            if (opt == 'Y' || opt == 'y')
            {
                // Add element to history array
                t[0].user_table.users[user_index].history[position_history] = product_status(res.id, SENT);
                ++position_history;
                ++t[0].user_table.users[user_index].history_len;
                bought[position] = res;
                ++position;

                // Remove from cart
                int temp[ALLOC_SIZE];
                memoset(temp, ALLOC_SIZE, NIL);

                int j = 0;
                bool found = false;
                for (int i = 0; i < t[0].user_table.users[user_index].cart_len; ++i)
                {
                    if (t[0].user_table.users[user_index].cart[i] != res.id || found)
                        temp[j++] = t[0].user_table.users[user_index].cart[i];
                    else
                        found = true;
                }

                // Copy temp array back to the cart array
                for (int i = 0; i < ALLOC_SIZE; ++i)
                    t[0].user_table.users[user_index].cart[i] = temp[i];

                new_cart_len = t[0].user_table.users[user_index].cart_len - 1;
            }
            else if(opt == 'N' || opt == 'n')
            {
                ++index;
            }
        }
        t[0].user_table.users[user_index].cart_len = new_cart_len;
    }
    std::cout << "Your products will be sent soon..." << std::endl;

    generate_receipt(bought, t[0].user_table.users[t[0].current_user].name);

    return LOBBY;
}

void generate_receipt(Product p[], std::string client)
{
    int i = 0;
    std::time_t t = std::time(nullptr);
    std::string r = "receipt" + std::to_string(t) + ".txt";
    float sum = 0;

    std::ofstream f(r);
    if (f.is_open())
    {
        f << "RECEIPT" << std::endl;
        f << "Client: " << client << std::endl;

        while(p[i].id != NIL)
        {
            f << "Vendor: " << p[i].vendor << std::endl;

            f << p[i].id
              << ") "
              << p[i].name
              << " "
              << p[i].price
              << std::endl;
            sum += p[i].price;
            ++i;
        }
        f << "TOTAL: " << sum << std::endl;
    }
    else
        std::cout << "Error opening file" << std::endl;

    f.close();
    std::cout << "Receipt generated" << std::endl;
}

void generate_return_receipt(Product p[], std::string sender, std::string receiver)
{
    int i = 0;
    std::time_t t = std::time(nullptr);
    std::string r = "return_receipt" + std::to_string(t) + ".txt";

    std::ofstream f(r);
    if (f.is_open())
    {
        f << "RETURN RECEIPT" << std::endl;
        f << "Sender: " << sender << std::endl;
        f << "Receiver: " << receiver << std::endl;

        while (p[i].id != NIL)
        {
            f << p[i].id
              << ") "
              << p[i].name
              << " | Price: $"
              << p[i].price
              << " | Weight: "
              << p[i].weight
              << " kg | Dimensions: "
              << p[i].dimensions.x
              << " x "
              << p[i].dimensions.y
              << " x "
              << p[i].dimensions.z
              << std::endl;
            ++i;
        }
    }
    else
    {
        std::cout << "Error opening file" << std::endl;
    }

    f.close();
    std::cout << "Return receipt generated" << std::endl;
}

int return_product(Tables t[])
{
    int opt;
    int history_len = t[0].user_table.users[t[0].current_user - 1].history_len;
    int id;
    Product_Table p;
    Product rp[ALLOC_SIZE];

    std::cout << "RETURN MENU" << std::endl;

    std::cout << "Type the product's id you'd like to return" << std::endl;

    for (int i = 0 ; i < history_len; ++i)
    {
        id = break_composed_id(t[0].user_table.users[t[0].current_user - 1].history[i]);
        p.products[i] = find_product_table(std::to_string(id), t[0]);
        ++p.len;
    }
    print_products(p);


    while(true)
    {
        bool valid_id = false;

        std::cout << std::endl;
        std::cout << "-> ";
        std::cin >> opt;
        for (int i = 0 ; i < p.len ; ++i)
        {
            if (opt == p.products[i].id)
            {
                valid_id = true;
                id = p.products[i].id;
                rp[i] = p.products[i];
                break;
            }
        }

        if (valid_id)
            break;
        else
            std::cout << "Invalid ID" << std::endl;
    }

    for (int i = 0 ; i < t[0].user_table.users[t[0].current_user - 1].history_len; ++i)
    {
        if (break_composed_id(t[0].user_table.users[t[0].current_user - 1].history[i]) == id)
            t[0].user_table.users[t[0].current_user - 1].history[i] = product_status(id, RETURNED);
    }

    generate_return_receipt(rp, t[0].user_table.users[t[0].current_user - 1].name, rp[0].vendor);

    std::cout << "Returned..." << std::endl;
    return LOBBY;
}

int exit_program(Tables t[])
{
    write_user_table(t[0].user_table);
    write_product_table(t[0].product_table);
    return NIL;
}

int product_status(int id, int status)
{
    if (status == SENT)
        return id * 10;
    else
        return  std::abs((id * 10) + status);
}

int break_composed_id(int composed_id)
{
    return composed_id / 10;
}

void copy_user_array(User source[], User destination[], int size) {
    for (int i = 0; i < size; ++i) {
        destination[i] = source[i];
    }
}

void copy_product_array(Product source[], Product destination[], int size) {
    for (int i = 0; i < size; ++i) {
        destination[i] = source[i];
    }
}

int tokenizer_csv(std::string line, std::string dest[])
{
    std::string str;
    int position = 0;

    for (size_t i = 0 ; i < line.length() ; ++i)
    {
        if (line[i] != ',')
            str += line[i];
        else
        {
            dest[position] = str;
            ++position;
            str = "";
        }
    }

    return position;
}

int tokenizer_dot(std::string line, std::string dest[])
{
    std::string str;
    int position = 0;

    for (size_t i = 0 ; i < line.length() ; ++i)
    {
        if (line[i] != '.')
            str += line[i];
        else
        {
            dest[position] = str;
            ++position;
            str = "";
        }
    }

    return position;
}
int tokenizer_dot_int(std::string line, int dest[])
{
    std::string str;
    int position = 0;

    for (size_t i = 0 ; i < line.length() ; ++i)
    {
        if (line[i] != '.')
            str += line[i];
        else
        {
            dest[position] = std::stoi(str);
            ++position;
            str = "";
        }
    }

    return position;
}

std::string iatos(int arr[])
{
    std::string str;

    for (int i = 0 ; arr[i] != NIL; ++i)
        str += std::to_string(arr[i]) + '.';

    return str;
}

void stoia(std::string line, int dest[])
{
    for (size_t i = 0 ; i < line.length() ; ++i)
        dest[i] = line[i] - '0';
}

std::string satos(std::string arr[])
{
    std::string str;

    for (int i = 0 ; arr[i] != "" ; ++i)
        str += arr[i] + '.';

    return str;
}

int touch(std::string path, std::string headers)
{
    std::ofstream f(path);

    if (f.is_open())
    {
        f << headers << std::endl;
        f.close();
        return 0;
    }
    else
    {
        std::cout << "Error opening file" << std::endl;
        return NIL;
    }
}

bool file_exists(std::string path)
{
    std::ifstream f(path);
    return f.good();
}

void memoset(int arr[], int len, int val)
{
    for (int i = 0 ; i < len ; ++i)
        arr[i] = val;
}

void memoset_str (std::string arr[], int len, std::string val)
{
    for (int i = 0 ; i < len ; ++i)
        arr[i] = val;
}

char ask(std::string question)
{
    char opt;

    std::cout << question << std::endl;
    std::cout << "-> ";
    std::cin >> opt;

    return opt;
}

Tables set_default_values(Tables t)
{
    // Write three users
    t.user_table.users[0].id = 1;
    t.user_table.users[0].name = "Alice Smith";
    t.user_table.users[0].age = 25;
    t.user_table.users[0].email = "alice.smith@example.com";
    t.user_table.users[0].password = "password123";
    t.user_table.users[0].wishlist[0] = 1;
    t.user_table.users[0].wishlist[1] = 2;
    t.user_table.users[0].wishlist[2] = 3;
    t.user_table.users[0].cart[0] = 3;
    t.user_table.users[0].cart[1] = 2;
    t.user_table.users[0].history[0] = 30;
    t.user_table.users[0].is_null = false;


    t.user_table.users[1].id = 2;
    t.user_table.users[1].name = "Bob Johnson";
    t.user_table.users[1].age = 30;
    t.user_table.users[1].email = "bob.johnson@example.com";
    t.user_table.users[1].password = "password456";
    t.user_table.users[1].wishlist[0] = 4;
    t.user_table.users[1].wishlist[1] = 5;
    t.user_table.users[1].wishlist[2] = 6;
    t.user_table.users[1].cart[0] = 5;
    t.user_table.users[1].history[0] = 60;
    t.user_table.users[1].is_null = false;

    t.user_table.users[2].id = 3;
    t.user_table.users[2].name = "Charlie Brown";
    t.user_table.users[2].age = 22;
    t.user_table.users[2].email = "charlie.brown@example.com";
    t.user_table.users[2].password = "password789";
    t.user_table.users[2].wishlist[0] = 7;
    t.user_table.users[2].wishlist[1] = 8;
    t.user_table.users[2].wishlist[2] = 9;
    t.user_table.users[2].cart[0] = 7;
    t.user_table.users[2].history[0] = 80;
    t.user_table.users[2].is_null = false;

    t.user_table.len += 3;

    // Write three products
    t.product_table.products[0].id = 1;
    t.product_table.products[0].name = "Product A";
    t.product_table.products[0].price = 19.99;
    t.product_table.products[0].dimensions = {10.0, 5.0, 2.0};
    t.product_table.products[0].weight = 1.5;
    t.product_table.products[0].stars = 4.5;
    t.product_table.products[0].comments[0] = "Great product!";
    t.product_table.products[0].comments[1] = "Worth the price";
    t.product_table.products[0].comments[2] = "";
    t.product_table.products[0].vendor = "Vendor A";

    t.product_table.products[1].id = 2;
    t.product_table.products[1].name = "Product B";
    t.product_table.products[1].price = 29.99;
    t.product_table.products[1].dimensions = {15.0, 10.0, 5.0};
    t.product_table.products[1].weight = 2.0;
    t.product_table.products[1].stars = 4.0;
    t.product_table.products[1].comments[0] = "Good quality";
    t.product_table.products[1].comments[1] = "Satisfied";
    t.product_table.products[1].comments[2] = "";
    t.product_table.products[1].vendor = "Vendor B";

    t.product_table.products[2].id = 3;
    t.product_table.products[2].name = "Product C";
    t.product_table.products[2].price = 9.99;
    t.product_table.products[2].dimensions = {5.0, 3.0, 1.0};
    t.product_table.products[2].weight = 0.5;
    t.product_table.products[2].stars = 3.5;
    t.product_table.products[2].comments[0] = "Average product";
    t.product_table.products[2].comments[1] = "Okay for the price";
    t.product_table.products[2].comments[2] = "";
    t.product_table.products[2].vendor = "Vendor C";

    t.product_table.len += 3;

    return t;
}

