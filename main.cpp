#define CROW_MAIN
#define SHOPPING_CART_PATH_PREFIX "./shopping-cart/"
#define SHOPPING_CART_PATH_SUFFIX ".txt"
#define DATA_SEPARATOR '|'
#define SHOPPING_CART_USERNAME "test@example.com"
#define SHOPPING_CART_PASSWORD "1234"
#define CRITICAL_ERROR_STRING "CRITICAL_ERROR"

#include "crow_all.h"
#include <iostream>
#include <string>
using namespace std;

static string CRITICAL_ERROR_MESSAGE;

bool fileExists(const string& filePath) {
    // This code was sourced with help from:
    // https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
    const char* fileCString = filePath.c_str();

    if (FILE *file = fopen(fileCString, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

bool shoppingCartFileExists(const string& shoppingCartFilePath) {
    return fileExists(shoppingCartFilePath);
}

string generateShoppingCartUuid() {
    // It turns out generating UUIDs in a lower-level language like C++
    // is harder than you think without libraries. But this StackOverflow answer
    // is quite helpful:
    // https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library#answer-58467162

    // Why I chose this answer over others is:
    // a) It's not Windows-library dependent
    // b) It's not Boost-library dependent (while Crow uses Boost, it misses some of the libraries
    // needed for rand generation)
    // c) It follows POSIX standards
    // d) It's simple and easy to understand

    static random_device randomDevice;
    static mt19937 rng(randomDevice());

    uniform_int_distribution<int> dist(0, 15);

    // This is the list of characters that may be used in the UUID (aside from the dashes)
    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    string res;

    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return res;
}

string createShoppingCartFilePath(const string& shoppingCartUuid) {
    string fileUuid = generateShoppingCartUuid();
    string filePathPrefix = SHOPPING_CART_PATH_PREFIX;
    string filePathSuffix = SHOPPING_CART_PATH_SUFFIX;

    string filePath = filePathPrefix + fileUuid + filePathSuffix;

    return filePath;
}

class ShoppingCartProduct {
    unsigned short int id;
    string name;
    float price;
    unsigned short int quantity;

public:
    ShoppingCartProduct(unsigned short int id, string name, float price, unsigned short int quantity) {
        this->id = id;
        this->name = name;
        this->price = price;
        this->quantity = quantity;
    }

    unsigned short int getId() {
        return this->id;
    }

    string getName() {
        return this->name;
    }

    void setName(string name) {
        this->name = name;
    }

    float getPrice() {
        return this->price;
    }

    void setPrice(float price) {
        this->price = price;
    }

    int getQuantity() {
        return this->quantity;
    }

    void setQuantity(unsigned short int quantity) {
        this->quantity = quantity;
    }

    string toDataFileString() {
        string idString = std::to_string(this->getId());
        string priceString = std::to_string(this->getPrice());
        string quantityString = std::to_string(this->getQuantity());
        return idString + DATA_SEPARATOR + this->getName() + DATA_SEPARATOR + priceString + DATA_SEPARATOR + quantityString;
    }

    string to_string() {
        string idString = std::to_string(this->getId());
        string priceString = std::to_string(this->getPrice());
        string quantityString = std::to_string(this->getQuantity());
        return "Product ID: " + idString +  ", Name: " + this->getName() + ", Price: " + priceString + ", Quantity: " + quantityString;
    }
};

bool createShoppingCartFile(const string& shoppingCartUuid, ShoppingCartProduct& shoppingCartProduct) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

    fstream shoppingCartFile(shoppingCartFilePath.c_str());
    shoppingCartFile.open(shoppingCartFilePath, std::ios::out | std::ios::trunc);

    if(!shoppingCartFile.is_open()) {
        return false;
    }

    string productString = shoppingCartProduct.toDataFileString();

    shoppingCartFile << productString;

    shoppingCartFile.close();

    return true;
}

bool appendToShoppingCartFile(const string& shoppingCartUuid, ShoppingCartProduct& shoppingCartProduct) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

    if(!shoppingCartFileExists(shoppingCartFilePath)) {
        return false;
    }

    fstream shoppingCartFile(shoppingCartFilePath.c_str());
    shoppingCartFile.open(shoppingCartFilePath, std::ios::out | std::ios::app);

    if(!shoppingCartFile.is_open()) {
        return false;
    }

    string productString = shoppingCartProduct.toDataFileString();

    shoppingCartFile << "\n" << productString;

    shoppingCartFile.close();

    return true;
}

vector<ShoppingCartProduct> getShoppingCartFileContents(crow::response& Response, const string& shoppingCartUuid) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);
    vector<ShoppingCartProduct> shoppingCartProducts;

    if(!shoppingCartFileExists(shoppingCartFilePath)) {
        return shoppingCartProducts;
    }

    ifstream fileReader(shoppingCartFilePath.c_str());

    if(!fileReader.is_open()) {
        Response.code = crow::status::INTERNAL_SERVER_ERROR;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "ERROR: unable to retrieve your shopping cart, please try again later";
        Response.end();
        return shoppingCartProducts;
    }

    string currentLine;
    while(getline(fileReader, currentLine)) {
        string currentStringPart;
        istringstream currentLineStream(currentStringPart);
        int currentIndex = 0;

        // If the current length of this line is 0, then we won't count it
        if(currentLine.empty()) {
            currentIndex++;
            continue;
        }

        int productId;
        string productName;
        float productPrice;
        int productQuantity;
        bool invalidLine = false;

        while(getline(currentLineStream, currentStringPart, DATA_SEPARATOR)) {
            // We know for a fact that this line isn't empty, so it should be following the format
            // specified as [Product Name]|[Price]|Quantity. There should never be an empty part of
            // the string, so we've run into a critical error
            if(currentStringPart.empty()) {
                CRITICAL_ERROR_MESSAGE = "A line in the shopping cart with ID " + shoppingCartUuid + " is malformed and as such the application is stopping as a result. The index that was attempting to be read was: " + std::to_string(currentIndex) + " but it was empty.";
                vector<ShoppingCartProduct> emptyProducts;
                return emptyProducts;
            }

            switch(currentIndex) {
                case 0:
                    productId = atoi(currentStringPart.c_str());
                case 1:
                    productName = currentStringPart;
                    break;
                case 2:
                    productPrice = atof(currentStringPart.c_str());
                    break;
                case 3:
                    productQuantity = atoi(currentStringPart.c_str());
                    break;
                default:
                    invalidLine = true;
                    break;
            }

            if(!invalidLine) {
                shoppingCartProducts.emplace_back(productId, productName, productPrice, productQuantity);
            }

            currentIndex++;
        }
    }

    fileReader.close();

    return shoppingCartProducts;
}

string readShoppingCartAsRawAsciiString(const string& shoppingCartFilePath) {
    const char* shoppingCartFilePathAsCStr = shoppingCartFilePath.c_str();

    if(!shoppingCartFileExists(shoppingCartFilePathAsCStr)) {
        return "";
    }

    ifstream fileReader(shoppingCartFilePathAsCStr);

    if(!fileReader.is_open()) {
        CRITICAL_ERROR_MESSAGE = "Unable to open shopping cart file " + shoppingCartFilePath + " despite it existing";
        return CRITICAL_ERROR_STRING;
    }

    string finalOutput;
    string currentLine;

    int currentIndex = 0;
    while(getline(fileReader, currentLine)) {
        if(currentIndex > 0) {
            finalOutput.append("\n");
        }

        finalOutput.append(currentLine);

        currentIndex++;
    }

    return finalOutput;
}

string returnAllShoppingCartItemsAsAsciiString(crow::response& Response, const string& shoppingCartUuid, bool asRawAsciiString = false) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

    if(!shoppingCartFileExists(shoppingCartFilePath)) {
        return "No Products in Cart";
    }

    if(asRawAsciiString) {
        string outputContent = readShoppingCartAsRawAsciiString(shoppingCartFilePath);

        if(outputContent == CRITICAL_ERROR_STRING) {
            Response.code = crow::status::INTERNAL_SERVER_ERROR;
            Response.add_header("Content-Type", "text/plain");
            Response.body = "k";
        }
    }
}

bool removeCartItem(crow::response& Response, const string& shoppingCartUuid, unsigned short int productId) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

    if(!shoppingCartFileExists(shoppingCartFilePath)) {
        return false;
    }

    // Read the entire file into memory
    vector<ShoppingCartProduct> shoppingCartProducts = getShoppingCartFileContents(Response, shoppingCartFilePath);

    // If the cart is somehow empty, something has gone very wrong, and there's no way
    // we can delete a non-existent cart item, so we'll just return false
    // This case could be achieved if elsewhere in the code somehow only empty lines existed
    // instead of the file being deleted if it was empty
    if(shoppingCartProducts.empty()) {
        return false;
    }

    // Loop through and see if we can find an element whose searchName and searchQuantity matches
    // the provided parameters
    for (int i = 0; i < shoppingCartProducts.size(); i++) {
        ShoppingCartProduct currentProduct = shoppingCartProducts[i];

        // If the current product's name and quantity matches, erase it from the vector
        if(currentProduct.getId() == productId) {
            shoppingCartProducts.erase(shoppingCartProducts.begin() + i);
            break;
        }
    }

    // If the cart is now empty (we deleted the only product), we should delete the shopping cart file
    // We can also return since there's no more logic to be done
    if(shoppingCartProducts.empty()) {
        remove(shoppingCartFilePath.c_str());
        return true;
    }

    // For the first element, we'll recreate the file
    bool shoppingCartFileRecreatedSuccessfully = createShoppingCartFile(shoppingCartUuid, shoppingCartProducts.front());

    // If the file couldn't be created then we'll return false
    if(!shoppingCartFileRecreatedSuccessfully) {
        return false;
    }

    // For all subsequent items, we can append them to the file we've now created
    // Note that if there was only 1 element in the new array this code wouldn't run, so there is no
    // need for an if statement to check if there's only 1 element
    for(int i = 1; i < shoppingCartProducts.size(); i++) {
        ShoppingCartProduct currentProduct = shoppingCartProducts[i];

        bool appendedSuccessfully = appendToShoppingCartFile(shoppingCartUuid, currentProduct);

        // If the append command failed to add the new shopping cart line to the file, then
        // we've failed our operation
        if(!appendedSuccessfully) {
            return false;
        }
    }

    return true;
}

bool shoppingCartAuthenticationCheck(const string& inputUsername, const string& inputPassword) {
    const string correctUsername = SHOPPING_CART_USERNAME;
    const string correctPassword = SHOPPING_CART_PASSWORD;

    // The user successfully authenticated if they matched the hardcoded credentials on the server
    return (inputUsername == correctUsername) && (inputPassword == correctPassword);
}

bool shouldAppContinue(crow::SimpleApp& app) {
    bool appShouldContinue = CRITICAL_ERROR_MESSAGE.empty();

    if(!appShouldContinue) {
        app.stop();
    }

    cerr << "\n\n[CRITICAL ERROR] " << CRITICAL_ERROR_MESSAGE;
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
        .methods(crow::HTTPMethod::GET)
            ([&app](const crow::request& Request, crow::response& Response) {
                Response.body = "hi";
                Response.end();
            });

    app.port(23500).multithreaded().run();

    return 0;
}
