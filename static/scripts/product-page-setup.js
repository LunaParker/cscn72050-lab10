function generateBackRow(mainContainer) {
    let backRow = document.createElement('div');
    backRow.classList.add("product-back-row");
    mainContainer.appendChild(backRow);

    let backRowButton = document.createElement('a');
    backRowButton.setAttribute('href', '/');
    backRowButton.classList.add("button", "button--dark", "product-back-row__button");
    backRowButton.innerHTML = "&lt; Back to all products";
    backRow.appendChild(backRowButton);
}

function generateTopRow(mainContainer) {
    let productRow = document.createElement('section');
    productRow.setAttribute('id', 'top-product-row');
    productRow.classList.add("top-product-row");
    mainContainer.appendChild(productRow);

    let productRowContainer = document.createElement('div');
    productRowContainer.classList.add('top-product-row__container');
    productRow.appendChild(productRowContainer);

    /* IMAGE - LEFT ALIGNED */
    let productRowImageFlexContainer = document.createElement('div');
    productRowImageFlexContainer.classList.add('top-product-row__image-flex-container');
    productRowContainer.appendChild(productRowImageFlexContainer);

    let productRowImageContainer = document.createElement('div');
    productRowImageContainer.classList.add('top-product-row__image-container');
    productRowImageFlexContainer.appendChild(productRowImageContainer);

    let productImage = document.createElement('img');
    productImage.setAttribute('src', '/images/product-' + window.productId.toString() + '.jpg');
    productImage.setAttribute('alt', window.currentProduct.name);
    productRowImageContainer.appendChild(productImage);

    /* CONTENT - RIGHT ALIGNED */
    let productRowContentContainer = document.createElement('div');
    productRowContentContainer.classList.add('top-product-row__content-container');
    productRowContainer.appendChild(productRowContentContainer);

    let productTitleElement = document.createElement('h1');
    productTitleElement.innerHTML = window.currentProduct.name;
    productRowContentContainer.appendChild(productTitleElement);

    let productPriceElement = document.createElement('h2');
    productPriceElement.innerHTML = "$" + (window.currentProduct.price).toFixed(2).toString();
    productRowContentContainer.appendChild(productPriceElement);

    let productDescriptionElement = document.createElement('div');
    productDescriptionElement.classList.add('top-product-row__description');
    productDescriptionElement.innerHTML = window.currentProduct.description;
    productRowContentContainer.appendChild(productDescriptionElement);

    let invisibleFlexSpacer = document.createElement('div');
    invisibleFlexSpacer.classList.add('top-product-row__spacer');
    invisibleFlexSpacer.setAttribute('aria-hidden', 'true');
    productRowContentContainer.appendChild(invisibleFlexSpacer);

    let quantityAndCartForm = document.createElement('form');
    quantityAndCartForm.setAttribute('method', 'POST');
    quantityAndCartForm.setAttribute('action', "/cart/add/" + window.productId + "/" + window.currentProduct.slug);
    quantityAndCartForm.classList.add('top-product-row__form');
    productRowContentContainer.appendChild(quantityAndCartForm);

    let quantityContainer = document.createElement('div');
    quantityContainer.classList.add("top-product-row__form__quantity");
    quantityAndCartForm.appendChild(quantityContainer);

    let quantityLabel = document.createElement('label');
    quantityLabel.setAttribute('for', 'quantity-input');
    quantityLabel.innerHTML = "Quantity";
    quantityContainer.appendChild(quantityLabel);

    let quantityField = document.createElement('input');
    quantityField.setAttribute('id', 'quantity-input')
    quantityField.setAttribute('name', 'quantity');
    quantityField.setAttribute('type', 'number');
    quantityField.setAttribute('required', '');
    quantityField.setAttribute('min', '1');
    quantityField.setAttribute('max', '10');
    quantityField.setAttribute('value', '1');
    quantityField.classList.add('top-product-row__form__quantity');
    quantityContainer.appendChild(quantityField);

    let submitButton = document.createElement('button');
    submitButton.setAttribute('type', 'submit');
    submitButton.classList.add("button", "top-product-row__form__button");
    submitButton.innerHTML = "Add to Cart";
    quantityAndCartForm.appendChild(submitButton);
}

function createProductPageContent() {
    let mainContainer = document.getElementById('main-content');

    // If for some reason the main container doesn't exist, we'll stop here
    if(!mainContainer) {
        return;
    }

    /* BREADCRUMB/BACK ROW */
    generateBackRow(mainContainer);

    /* MAIN ROW */
    generateTopRow(mainContainer);
}

function getProductId() {
    let pathFragments = window.location.pathname.split("/");

    // The second last fragment (the integer) represents the product ID
    if(pathFragments.length < 3) {
        return;
    }

    return parseInt(pathFragments[pathFragments.length - 2]);
}

document.addEventListener("DOMContentLoaded", (event) => {
    window.productId = getProductId();

    if(!window.productId) {
        console.error("Could not determine the product ID based on the path");
        return;
    }

    window.currentProduct = products[window.productId.toString()];

    createProductPageContent();
});