// Although I'm decent at Javascript, there's no way I could
// make a function as clean and efficient at getting cookies as
// the one below:
// https://stackoverflow.com/questions/5639346/what-is-the-shortest-function-for-reading-a-cookie-by-name-in-javascript#answer-25490531
const getCookieValue = (name) => (
    document.cookie.match('(^|;)\\s*' + name + '\\s*=\\s*([^;]+)')?.pop() || ''
)

document.addEventListener("DOMContentLoaded", (event) => {
    alert('Your purchase was completed successfully.');
    window.location.pathname = "/cart/" + getCookieValue("shopping_cart_id") + "/raw";
});
