#define CROW_MAIN
#define SHOPPING_CART_DIRECTORY "./shopping-cart"
#define SHOPPING_CART_PATH_PREFIX "./shopping-cart/"
#define SHOPPING_CART_PATH_SUFFIX ".txt"
#define DATA_SEPARATOR '|'
#define SHOPPING_CART_USERNAME "test@example.com"
#define SHOPPING_CART_PASSWORD "1234"
#define RELATIVE_PUBLIC_PATH "../static/"
#define CART_COOKIE_NAME "shopping_cart_id"

#include "crow_all.h"
#include <iostream>
#include <string>
#include <regex>
#include <filesystem>
using namespace std;

static string CRITICAL_ERROR_MESSAGE;

bool isValidUnsignedIntegerFromString(const char* validationString) {
    while(*validationString != '\0') {
        if(!isdigit(*validationString)) {
            return false;
        }

        validationString++;
    }

    return true;
}

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

string getPublicPath() {
    string filePathString = RELATIVE_PUBLIC_PATH;
    return filePathString;
}

bool isValidFileName(const string& possibleFileName) {
    regex fileNamePattern(R"(^(.*)(\.\.)(.*)$)");

    return !regex_match(possibleFileName, fileNamePattern);
}

void runPublicStaticResourceActions(const crow::request& Request, crow::response& Response, const string& resourceType, const string& inputResourceFileName) {
    // First we'll do validation to make sure the user hasn't provided any invalid
    // characters which could grant them access to paths we don't want them to

    if(!isValidFileName(inputResourceFileName)) {
        Response.code = crow::status::BAD_REQUEST;
        Response.end();
        return;
    }

    // Now that we've validated we have a good file name, we can construct the relative path to the file
    string filePath = getPublicPath() + resourceType + '/' + inputResourceFileName;

    // We'll check if it exists - if it doesn't exist, we'll return a 404
    if(!fileExists(filePath)) {
        Response.code = crow::status::NOT_FOUND;
        Response.end();
        return;
    }

    // All is good, we can now return our static asset
    Response.code = crow::status::OK;
    Response.set_static_file_info_unsafe(filePath);
    Response.end();
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
    string filePathPrefix = SHOPPING_CART_PATH_PREFIX;
    string filePathSuffix = SHOPPING_CART_PATH_SUFFIX;

    string filePath = filePathPrefix + shoppingCartUuid + filePathSuffix;

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
    const string shoppingCartDirectory = SHOPPING_CART_DIRECTORY;

    if(!filesystem::exists(shoppingCartDirectory)) {
        try {
            filesystem::create_directory(shoppingCartDirectory);
        } catch (...) {
            return false;
        }
    }

    const string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

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

    ofstream shoppingCartFile(shoppingCartFilePath.c_str(), std::ios::out | std::ios::app);

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
        return "";
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

void returnAllShoppingCartItemsAsAsciiString(const crow::request& Request, crow::response& Response, const string& shoppingCartUuid) {
    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);
    Response.add_header("Content-Type", "text/plain");

    if(!shoppingCartFileExists(shoppingCartFilePath)) {
        Response.code = crow::status::NOT_FOUND;
        Response.body = "The shopping cart file associated with your session doesn't exist. The shopping cart path is: " + shoppingCartFilePath;
        return;
    }

    string outputContent = readShoppingCartAsRawAsciiString(shoppingCartFilePath);

    if(outputContent.empty()) {
        Response.code = crow::status::INTERNAL_SERVER_ERROR;
        Response.body = "Unable to open the given shopping cart file: " + shoppingCartFilePath;
        return;
    }

    Response.code = crow::status::ACCEPTED;
    Response.body = outputContent;
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

void runAddToCartLogic(const crow::request& Request, crow::response& Response, std::unordered_map<unsigned short int, ShoppingCartProduct> products, const string& shoppingCartUuid, int providedProductId, const string& providedProductName) {
    if(shoppingCartUuid.empty()) {
        Response.code = crow::status::FORBIDDEN;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "You did not provide your cart's unique ID.";
        Response.end();
        return;
    }

    string productPagePath = getPublicPath() + "product-" + std::to_string(providedProductId) + ".html";

    if(!Request.get_body_params().get("quantity") || !fileExists(productPagePath) || !isValidUnsignedIntegerFromString(Request.get_body_params().get("quantity"))) {
        Response.code = crow::status::BAD_REQUEST;
        Response.add_header("Content-Type", "text/plain");

        if(!Request.get_body_params().get("quantity") || !isValidUnsignedIntegerFromString(Request.get_body_params().get("quantity"))) {
            Response.body = "You did not provide a valid quantity as part of your POST request.";
        } else {
            Response.body = "You provided an invalid product.";
        }

        Response.end();
        return;
    }

    string shoppingCartFilePath = createShoppingCartFilePath(shoppingCartUuid);

    bool doesShoppingCartFileExist = shoppingCartFileExists(shoppingCartFilePath);

    regex encodedUrlSpaceRegex("%20");
    string finalProductName = std::regex_replace(providedProductName, encodedUrlSpaceRegex, " ");

    int productQuantity = atoi(Request.get_body_params().get("quantity"));
    ShoppingCartProduct matchingProduct = products.at(providedProductId);
    ShoppingCartProduct newProduct = ShoppingCartProduct((unsigned short int) providedProductId, finalProductName, matchingProduct.getPrice(), productQuantity);

    bool addedToCartSuccess = false;

    if(doesShoppingCartFileExist) {
        addedToCartSuccess = appendToShoppingCartFile(shoppingCartUuid, newProduct);
    } else {
        addedToCartSuccess = createShoppingCartFile(shoppingCartUuid, newProduct);
    }

    if(!addedToCartSuccess) {
        Response.code = crow::status::INTERNAL_SERVER_ERROR;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "Unfortunately, we were unable to add your product to your cart. Please try again later.";
        Response.end();
        return;
    }

    // Now we redirect them to the login route (which is just the cart route,
    // but as a GET request)
    Response.code = crow::status::FOUND;
    Response.add_header("Location", "/cart");
    Response.end();
}

void runProductPageLogic(const crow::request& Request, crow::response& Response, int providedProductId) {
    // First we'll check if it's a valid product ID by seeing if a corresponding
    // product page exists
    // This is to allow for the addition of more products in future
    string htmlFilePath = getPublicPath() + '/' + "product-" + std::to_string(providedProductId) + ".html";

    // We'll check if it exists - if it doesn't exist, we'll return a 404
    if(!fileExists(htmlFilePath)) {
        Response.code = crow::status::NOT_FOUND;
        Response.end();
        return;
    }

    Response.code = crow::status::OK;
    Response.set_static_file_info_unsafe(htmlFilePath);
    Response.add_header("Content-Type", "text/html");
    Response.end();
}

bool shoppingCartAuthenticationCheck(const string& inputUsername, const string& inputPassword) {
    const string correctUsername = SHOPPING_CART_USERNAME;
    const string correctPassword = SHOPPING_CART_PASSWORD;

    // The user successfully authenticated if they matched the hardcoded credentials on the server
    return (inputUsername == correctUsername) && (inputPassword == correctPassword);
}

void runCartRawAsciiGetRequest(crow::response& Response, bool authenticated, const string& shoppingCartUuid) {
    Response.add_header("Content-Type", "text/plain");

    if(!authenticated) {
        Response.code = crow::status::UNAUTHORIZED;
        Response.body = "You must be authenticated to view this page.";
        Response.end();
        return;
    }

    Response.code = crow::status::OK;
    string shoppingCartRawContents = readShoppingCartAsRawAsciiString(createShoppingCartFilePath(shoppingCartUuid));
    Response.body = shoppingCartRawContents;
    Response.end();
}

void runCartPaymentSuccessPage(const crow::request& Request, crow::response& Response, bool authenticated, const string& shoppingCartUuid) {
    if(!authenticated) {
        Response.code = crow::status::UNAUTHORIZED;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "You must be authenticated to view this page.";
        Response.end();
        return;
    }

    const string potentialShoppingCartPath = createShoppingCartFilePath(shoppingCartUuid);

    if(!shoppingCartFileExists(potentialShoppingCartPath)) {
        Response.code = crow::status::NOT_FOUND;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "The shopping cart you've provided does not exist";
    }

    Response.code = crow::status::ACCEPTED;
    string filePath = getPublicPath() + "payment-success.html";
    Response.set_static_file_info_unsafe(filePath);
    Response.add_header("Content-Type", "text/html");
    Response.end();
}

void runCartLoginPage(const crow::request& Request, crow::response& Response, bool shouldShowError = false) {
    if(shouldShowError) {
        Response.code = crow::status::UNAUTHORIZED;
    } else {
        Response.code = 402;
    }

    string filePath = getPublicPath() + "login.html";
    Response.set_static_file_info_unsafe(filePath);
    Response.add_header("Content-Type", "text/html");
    Response.end();
}

bool runCartAuthenticationLogic(const crow::request& Request, crow::response& Response, const string& shoppingCartUuid) {
    // First we need to make sure the user provided all necessary
    // info in their post request
    if(!Request.get_body_params().get("username") || !Request.get_body_params().get("password")) {
        Response.code = crow::status::BAD_REQUEST;
        Response.add_header("Content-Type", "text/plain");
        Response.body = "Your login attempt is missing all necessary fields";
        Response.end();
        return false;
    }

    bool correctLogin = shoppingCartAuthenticationCheck(Request.get_body_params().get("username"), Request.get_body_params().get("password"));

    if(!correctLogin) {
        runCartLoginPage(Request, Response, true);
        return false;
    }

    Response.code = crow::status::FOUND;
    Response.add_header("Location", "/cart/" + shoppingCartUuid + "/payment/success");
    return true;
}

bool runCartPageLogic(const crow::request& Request, crow::response& Response, const string& shoppingCartUuid) {
    switch(Request.method) {
        case crow::HTTPMethod::GET:
            runCartLoginPage(Request, Response);
            return false;
        case crow::HTTPMethod::POST:
            return runCartAuthenticationLogic(Request, Response, shoppingCartUuid);
        default:
            Response.code = crow::status::METHOD_NOT_ALLOWED;
            Response.end();
            return false;
    }
}

struct shoppingCartCookieParser : public crow::CookieParser {
    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        crow::CookieParser::before_handle(req, res, ctx);

        const string shoppingCartCookieName = CART_COOKIE_NAME;

        if(ctx.get_cookie(shoppingCartCookieName).empty()) {
            string newShoppingCartUuid = generateShoppingCartUuid();
            ctx.set_cookie(shoppingCartCookieName, newShoppingCartUuid)
                .path("/");
        }
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx)
    {
        crow::CookieParser::after_handle(req, res, ctx);
    }
};

int main()
{
    crow::App<shoppingCartCookieParser> app;

    ShoppingCartProduct productOne = ShoppingCartProduct(1, "GO Engineer Cap", 19.95, 0);
    ShoppingCartProduct productTwo = ShoppingCartProduct(2, "Metrolinx Tumbler", 35.56, 0);
    ShoppingCartProduct productThree = ShoppingCartProduct(3, "PRESTO Laptop Case", 30.50, 0);
    ShoppingCartProduct productFour = ShoppingCartProduct(4, "PRESTO T-shirt", 20.00, 0);

    std::unordered_map<unsigned short int, ShoppingCartProduct> products = {
            {1, productOne},
            {2, productTwo},
            {3, productThree},
            {4, productFour}
    };

    CROW_ROUTE(app, "/products/<int>/<string>")
        .methods(crow::HTTPMethod::GET)
                ([](const crow::request& Request, crow::response& Response, int inputProductId, string inputProductNamePath) {
                    runProductPageLogic(Request, Response, inputProductId);
                });

    CROW_ROUTE(app, "/cart")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)
            ([&](const crow::request& Request, crow::response& Response) {
                const string shoppingCartCookieName = CART_COOKIE_NAME;
                auto& ctx = app.get_context<shoppingCartCookieParser>(Request);
                string shoppingCartUuid = ctx.get_cookie(shoppingCartCookieName);

                bool successfullyAuthenticated = runCartPageLogic(Request, Response, shoppingCartUuid);

                if(successfullyAuthenticated && Request.method == crow::HTTPMethod::POST) {
                    ctx.set_cookie("authenticated", "true")
                        .path("/");
                    Response.end();
                }
            });

    CROW_ROUTE(app, "/cart/<string>/add/<int>/<string>")
        .methods(crow::HTTPMethod::POST)
                ([&](const crow::request& Request, crow::response& Response, string shoppingCartUuid, int providedProductId, string providedProductName) {
                    auto& ctx = app.get_context<shoppingCartCookieParser>(Request);
                    ctx.set_cookie("cart_filled", "true")
                        .path("/");

                    runAddToCartLogic(Request, Response, products, shoppingCartUuid, providedProductId, providedProductName);
                });

    CROW_ROUTE(app, "/cart/<string>/raw")
        .methods(crow::HTTPMethod::GET)
                ([&](const crow::request& Request, crow::response& Response, string shoppingCartUuid) {
                    auto& ctx = app.get_context<shoppingCartCookieParser>(Request);
                    string authenticatedCookie = ctx.get_cookie("authenticated");
                    bool authenticationSuccessful = (authenticatedCookie == "true");

                    runCartRawAsciiGetRequest(Response, authenticationSuccessful, shoppingCartUuid);
                });

    CROW_ROUTE(app, "/cart/<string>/payment/success")
        .methods(crow::HTTPMethod::GET)
                ([&](const crow::request& Request, crow::response& Response, string shoppingCartUuid) {
                    auto& ctx = app.get_context<shoppingCartCookieParser>(Request);
                    string authenticatedCookie = ctx.get_cookie("authenticated");
                    bool authenticationSuccessful = (authenticatedCookie == "true");

                    runCartPaymentSuccessPage(Request, Response, authenticationSuccessful, shoppingCartUuid);
                });

    CROW_ROUTE(app, "/images/<string>")
        .methods(crow::HTTPMethod::GET)
            ([](const crow::request& Request, crow::response& Response, string inputImageFileName) {
                runPublicStaticResourceActions(Request, Response, "images", inputImageFileName);
            });

    CROW_ROUTE(app, "/scripts/<string>")
        .methods(crow::HTTPMethod::GET)
            ([](const crow::request& Request, crow::response& Response, string inputScriptFileName) {
                runPublicStaticResourceActions(Request, Response, "scripts", inputScriptFileName);
            });

    CROW_ROUTE(app, "/styles/<string>")
        .methods(crow::HTTPMethod::GET)
            ([](const crow::request& Request, crow::response& Response, string inputStyleFileName) {
                runPublicStaticResourceActions(Request, Response, "styles", inputStyleFileName);
            });

    CROW_CATCHALL_ROUTE(app)
        ([](const crow::request& Request, crow::response& Response) {
            Response.code = crow::status::OK;
            string filePath = getPublicPath() + "index.html";
            Response.set_static_file_info_unsafe(filePath);
            Response.add_header("Content-Type", "text/html");
            Response.end();
        });

    app.port(23500).multithreaded().run();

    return 0;
}
