function generateProductCard(currentProductId, currentProductObject, productsGrid) {
    let productCard = document.createElement('a');
    productCard.setAttribute('href', getProductLink(currentProductId, currentProductObject));
    productCard.classList.add("index-product");
    productsGrid.appendChild(productCard);

    /* TOP IMAGE */
    let productImageContainer = document.createElement('div');
    productImageContainer.classList.add("index-product__image-container");
    productCard.appendChild(productImageContainer);

    let productImage = document.createElement('img');
    productImage.setAttribute('src', "/images/product-" + currentProductId + ".jpg");
    productImage.setAttribute('alt', currentProductObject.name);
    productImageContainer.appendChild(productImage);

    /* CARD CONTENT */
    let productContentContainer = document.createElement('div');
    productContentContainer.classList.add("index-product__content");
    productCard.appendChild(productContentContainer);

    let productContentTitle = document.createElement('h2');
    productContentTitle.classList.add("index-product__content__title");
    productContentTitle.innerHTML = currentProductObject.name;
    productContentContainer.appendChild(productContentTitle);

    let productContentButton = document.createElement('div');
    productContentButton.classList.add("button", "index-product__content__button");
    productContentButton.innerHTML = "View Product";
    productContentContainer.appendChild(productContentButton);
}

function generateProductsGrid() {
    // First we'll establish the container we'll be putting the products into
    let productGridContainer = document.getElementById('index-products');

    // If for some reason it doesn't exist we can't inject into it
    if(!productGridContainer) {
        console.error("Unable to find #index-products on the page, does it exist?");
        return;
    }

    for (const currentProductId in products) {
        let currentProductObject = products[currentProductId];

        generateProductCard(currentProductId, currentProductObject, productGridContainer);
    }
}

document.addEventListener("DOMContentLoaded", (event) => {
    // If we don't have products injected, the rest of the generation code
    // won't work, so we'll just return an error
    if(!products) {
        console.error("Failed to load products for index grid - did you include products.js?");
        return;
    }

    generateProductsGrid();
});
